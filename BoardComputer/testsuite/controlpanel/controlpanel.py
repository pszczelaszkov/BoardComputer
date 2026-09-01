'''
Panel to apply values to BoardComputer during x86 run.

Writes ADC values to BC_DIR/ADCn, IGNITION state to BC_DIR/IGNITION, and drives
fuel/speed counters and keys via SIGRT.
'''
import os

os.environ.setdefault('KIVY_NO_ARGS', '1')

import argparse
import ctypes
import ctypes.util
import errno
import signal
import sys
from dataclasses import dataclass

from kivy.app import App
from kivy.clock import Clock
from kivy.core.window import Window
from kivy.uix.boxlayout import BoxLayout

ADC_CHANNEL_COUNT = 8
ADC_MAX = 1023
UINT16_MAX = 65535

COUNTERS_SIG_FUEL = signal.SIGRTMIN + 0
COUNTERS_SIG_SPEED = signal.SIGRTMIN + 1
INPUT_SIG = signal.SIGRTMIN + 3

INPUT_KEY_ENTER = 0
INPUT_KEY_DOWN = 1

ADC_LABELS = (
    'OILTEMP',
    'INTAKETEMP',
    'OUTTEMP',
    'MAP',
    'FRP',
    'TANK',
    'EGT',
    'unused',
)


@dataclass
class PanelConfig:
    verbose: bool
    bc_dir: str
    pid: int | None


def _parse_uint(text, maximum):
    '''Parse non-negative int from TextInput; empty/invalid -> 0, clamp to max.'''
    if not text:
        return 0
    try:
        value = int(text)
    except ValueError:
        return 0
    if value < 0:
        return 0
    if value > maximum:
        return maximum
    return value


def _log_error(message):
    print(message, file=sys.stderr)


if hasattr(signal, 'sigqueue'):
    def _sigqueue(pid, signo, value):
        signal.sigqueue(pid, signo, value)
else:
    _libc = ctypes.CDLL(ctypes.util.find_library('c'), use_errno=True)

    class _Sigval(ctypes.Union):
        _fields_ = [
            ('sival_int', ctypes.c_int),
            ('sival_ptr', ctypes.c_void_p),
        ]

    _libc.sigqueue.argtypes = [ctypes.c_int, ctypes.c_int, _Sigval]
    _libc.sigqueue.restype = ctypes.c_int

    def _sigqueue(pid, signo, value):
        payload = _Sigval()
        payload.sival_int = int(value)
        if _libc.sigqueue(pid, signo, payload) != 0:
            err = ctypes.get_errno()
            raise OSError(err, os.strerror(err))


def _resolve_boardcomputer_pid():
    '''Return first PID whose comm is BoardComputer, or None.'''
    matches = []
    try:
        for entry in os.scandir('/proc'):
            if not entry.name.isdigit():
                continue
            try:
                with open(os.path.join(entry.path, 'comm'), encoding='ascii') as comm_file:
                    comm = comm_file.read().strip()
            except OSError:
                continue
            if comm == 'BoardComputer':
                matches.append(int(entry.name))
    except OSError:
        return None

    if not matches:
        return None
    if len(matches) > 1:
        _log_error(
            f'warning: multiple BoardComputer processes found {matches}; '
            f'using {matches[0]}'
        )
    return matches[0]


def _parse_args(argv):
    default_bc_dir = os.environ.get('BC_DIR') or '.'

    parser = argparse.ArgumentParser(
        description='BoardComputer manual test control panel (ADC files + SIGRT).'
    )
    parser.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='log ADC/counter state changes and SIGRT clock start/stop',
    )
    parser.add_argument(
        '--bc-dir',
        default=default_bc_dir,
        help='directory for ADC0..ADC7 and IGNITION (default: $BC_DIR or .)',
    )
    parser.add_argument(
        '--pid',
        type=int,
        default=None,
        help='BoardComputer PID for sigqueue (default: auto-detect via /proc)',
    )
    return parser.parse_args(argv)


class ControlPanelRoot(BoxLayout):
    def __init__(self, config: PanelConfig, **kwargs):
        self.config = config
        self.ignition_enabled = self._read_ignition_file()
        super().__init__(**kwargs)
        self.adc_values = [0] * ADC_CHANNEL_COUNT
        self.injector_value = 0
        self.injector_interval = 0
        self.speed_value = 0
        self.speed_interval = 0
        self._fuel_clock = None
        self._speed_clock = None
        self._fuel_clock_error_logged = False
        self._speed_clock_error_logged = False
        self._keys_error_logged = False

    def _read_ignition_file(self):
        path = os.path.join(self.config.bc_dir, 'IGNITION')
        try:
            with open(path, encoding='ascii') as ignition_file:
                text = ignition_file.read().strip()
        except OSError:
            return True
        if not text:
            return True
        try:
            return int(text) != 0
        except ValueError:
            return True

    def _write_ignition_file(self, enabled):
        if not self._ensure_bc_dir():
            return

        path = os.path.join(self.config.bc_dir, 'IGNITION')
        value = 1 if enabled else 0
        try:
            with open(path, 'w', encoding='ascii') as ignition_file:
                ignition_file.write(f'{value}\n')
        except OSError as exc:
            _log_error(f'IGNITION write failed ({path}): {exc}')
            return

        self._log_verbose(f'IGNITION = {value}')

    def on_ignition_change(self, active):
        enabled = bool(active)
        if self.ignition_enabled == enabled:
            return
        self.ignition_enabled = enabled
        self._write_ignition_file(enabled)

    def _log_verbose(self, message):
        if self.config.verbose:
            print(message)

    def _ensure_bc_dir(self):
        try:
            os.makedirs(self.config.bc_dir, exist_ok=True)
        except OSError as exc:
            _log_error(f'cannot create bc-dir {self.config.bc_dir!r}: {exc}')
            return False
        return True

    def _write_adc_file(self, index, value):
        if not self._ensure_bc_dir():
            return

        path = os.path.join(self.config.bc_dir, f'ADC{index}')
        try:
            with open(path, 'w', encoding='ascii') as adc_file:
                adc_file.write(f'{value}\n')
        except OSError as exc:
            _log_error(f'ADC{index} write failed ({path}): {exc}')
            return

        label = ADC_LABELS[index]
        self._log_verbose(f'ADC{index} ({label}) = {value}')

    def on_adc_change(self, index, value):
        clamped = int(max(0, min(ADC_MAX, value)))
        if self.adc_values[index] == clamped:
            return
        self.adc_values[index] = clamped
        self._write_adc_file(index, clamped)

    def _sigqueue_pulse(self, signo, value, clock_name, error_flag_attr):
        pid = self.config.pid
        if pid is None:
            if not getattr(self, error_flag_attr):
                _log_error(
                    f'{clock_name}: no BoardComputer PID; '
                    'use --pid or start BoardComputer first'
                )
                setattr(self, error_flag_attr, True)
            return False

        try:
            _sigqueue(pid, signo, value)
        except ProcessLookupError:
            if not getattr(self, error_flag_attr):
                _log_error(f'{clock_name}: process {pid} not found')
                setattr(self, error_flag_attr, True)
            return False
        except PermissionError:
            if not getattr(self, error_flag_attr):
                _log_error(f'{clock_name}: permission denied sending to PID {pid}')
                setattr(self, error_flag_attr, True)
            return False
        except OSError as exc:
            if exc.errno == errno.ESRCH:
                if not getattr(self, error_flag_attr):
                    _log_error(f'{clock_name}: process {pid} not found')
                    setattr(self, error_flag_attr, True)
                return False
            if exc.errno == errno.EPERM:
                if not getattr(self, error_flag_attr):
                    _log_error(f'{clock_name}: permission denied sending to PID {pid}')
                    setattr(self, error_flag_attr, True)
                return False
            if not getattr(self, error_flag_attr):
                _log_error(f'{clock_name}: sigqueue failed: {exc}')
                setattr(self, error_flag_attr, True)
            return False

        setattr(self, error_flag_attr, False)
        return True

    def _fuel_tick(self, _dt):
        if not self._sigqueue_pulse(
            COUNTERS_SIG_FUEL,
            self.injector_value,
            'injector',
            '_fuel_clock_error_logged',
        ):
            self._stop_fuel_clock(log_stop=False)

    def _speed_tick(self, _dt):
        if not self._sigqueue_pulse(
            COUNTERS_SIG_SPEED,
            self.speed_value,
            'speed',
            '_speed_clock_error_logged',
        ):
            self._stop_speed_clock(log_stop=False)

    def _stop_fuel_clock(self, log_stop=True):
        if self._fuel_clock is not None:
            Clock.unschedule(self._fuel_clock)
            self._fuel_clock = None
            if log_stop:
                self._log_verbose('injector SIGRT clock stopped')

    def _stop_speed_clock(self, log_stop=True):
        if self._speed_clock is not None:
            Clock.unschedule(self._speed_clock)
            self._speed_clock = None
            if log_stop:
                self._log_verbose('speed SIGRT clock stopped')

    def _update_fuel_clock(self):
        self._stop_fuel_clock(log_stop=False)
        if self.injector_interval <= 0:
            self._log_verbose('injector SIGRT clock stopped')
            return

        interval_s = self.injector_interval / 1000.0
        self._fuel_clock = Clock.schedule_interval(self._fuel_tick, interval_s)
        self._log_verbose(
            f'injector SIGRT clock started: value={self.injector_value}, '
            f'interval={self.injector_interval} ms'
        )

    def _update_speed_clock(self):
        self._stop_speed_clock(log_stop=False)
        if self.speed_interval <= 0:
            self._log_verbose('speed SIGRT clock stopped')
            return

        interval_s = self.speed_interval / 1000.0
        self._speed_clock = Clock.schedule_interval(self._speed_tick, interval_s)
        self._log_verbose(
            f'speed SIGRT clock started: value={self.speed_value}, '
            f'interval={self.speed_interval} ms'
        )

    def on_counter_field(self, name, text):
        if name.endswith('_interval'):
            parsed = _parse_uint(text, 2**31 - 1)
        else:
            parsed = _parse_uint(text, UINT16_MAX)

        current = getattr(self, name, None)
        if current == parsed:
            return
        setattr(self, name, parsed)
        self._log_verbose(f'{name} = {parsed}')

        if name == 'injector_interval':
            self._update_fuel_clock()
        elif name == 'speed_interval':
            self._update_speed_clock()

    def on_key(self, key):
        if self._sigqueue_pulse(
            INPUT_SIG,
            key,
            'keys',
            '_keys_error_logged',
        ):
            key_name = 'ENTER' if key == INPUT_KEY_ENTER else 'DOWN'
            self._log_verbose(f'key edge: {key_name} ({key})')


class ControlPanelApp(App):
    title = 'BoardComputer Control Panel'

    def __init__(self, config: PanelConfig, **kwargs):
        super().__init__(**kwargs)
        self.panel_config = config

    def build(self):
        Window.size = (900, 520)
        return ControlPanelRoot(config=self.panel_config)


def main():
    args = _parse_args(sys.argv[1:])

    bc_dir = os.path.abspath(args.bc_dir)
    pid = args.pid
    if pid is None:
        pid = _resolve_boardcomputer_pid()

    config = PanelConfig(verbose=args.verbose, bc_dir=bc_dir, pid=pid)

    if config.verbose:
        print('ControlPanel started')
        print(f'bc-dir: {config.bc_dir}')
        if config.pid is not None:
            print(f'pid: {config.pid}')
        else:
            print('pid: not found (SIGRT disabled until BoardComputer is running)')

    ControlPanelApp(config=config).run()


if __name__ == '__main__':
    main()
