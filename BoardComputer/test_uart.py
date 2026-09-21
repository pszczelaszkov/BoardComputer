import pytest
from helpers import (
    load,
    ModuleWrapper,
    uart_write_message,
    uart_consume_message,
    uart_send_nextion,
    uart_consume_nextion,
    read_nextion_output,
    read_service_output,
)

m, ffi = load("testmodule")
session = ModuleWrapper(m)


def uart_feed(channel, data):
    for byte in data:
        m.UART_put_byte_to_RX_buffer(channel, byte)


def uart_get(channel):
    message = ffi.new("uint8_t **")
    timestamp = ffi.new("SYSTEM_cycle_timestamp_t *")
    length = m.UART_get_next_message(channel, message, timestamp)
    if length == 0:
        assert message[0] == ffi.NULL
        return None
    return message[0], bytes(ffi.unpack(message[0], length))


class TestParent:
    @classmethod
    def setup_class(cls):
        m.SYSTEM_run = False
        m.CONFIG_factory_default_reset()
        m.test()
        m.NEXTION_handler_ready(m.NEXTION_VERSION)
        session.create_snapshot()

    @pytest.fixture(autouse=True)
    def snapshot_control(self):
        session.load_snapshot()
        m.UART_init()
        m.SERIAL_NEXTION_OUT_status = m.SERIAL_OUT_STATUS_BUSY
        m.SERIAL_SERVICE_OUT_status = m.SERIAL_OUT_STATUS_BUSY
        yield


class TestUARTNextionTXCapacity(TestParent):
    def test_max_descriptors_drops(self):
        # Keep payload tiny so the byte buffer is not the limiting factor
        payload = b"x"
        framed_len = len(payload) + m.UART_NEXTION_TERMINATOR_LEN
        count = m.UART_NEXTION_TX_MSG_DESCRIPTORS_COUNT
        assert count * framed_len <= m.UART_NEXTION_TX_BUFFER_SIZE

        for i in range(count):
            assert uart_send_nextion(m, ffi, payload) == 1, f"enqueue {i} should succeed"

        assert uart_send_nextion(m, ffi, payload) == 0

    def test_max_buffer_drops_with_descriptors_free(self):
        payload_len = 20
        payload = bytes([0x41 + (i % 26) for i in range(payload_len)])
        framed_len = payload_len + m.UART_NEXTION_TERMINATOR_LEN
        expected_ok = m.UART_NEXTION_TX_BUFFER_SIZE // framed_len

        assert expected_ok < m.UART_NEXTION_TX_MSG_DESCRIPTORS_COUNT

        for i in range(expected_ok):
            assert uart_send_nextion(m, ffi, payload) == 1, f"enqueue {i} should succeed"

        assert uart_send_nextion(m, ffi, payload) == 0

        # Buffer was the limit, not descriptors: free head space and wrap another payload in
        uart_consume_nextion(m)
        assert uart_send_nextion(m, ffi, payload) == 1

    def test_terminator_appended(self):
        m.SERIAL_NEXTION_OUT_status = m.SERIAL_OUT_STATUS_IDLE
        assert uart_send_nextion(m, ffi, b"x") == 1
        assert m.nextion_tx_message_length == 1 + m.UART_NEXTION_TERMINATOR_LEN
        assert bytes(ffi.unpack(m.nextion_tx_message, m.nextion_tx_message_length)) == b"x\xff\xff\xff"
        output = read_nextion_output(m, ffi)
        assert "x" in output


class TestUARTNextionTXWrap(TestParent):
    def test_wrap_to_front_after_consume(self):
        eot = m.UART_NEXTION_TERMINATOR_LEN
        # Framed sizes: 80 + 48 = 128
        first = bytes([0x11] * (80 - eot))
        second = bytes([0x22] * (48 - eot))
        wrapped = bytes([0x33] * (50 - eot))
        follow = bytes([0x44] * (20 - eot))

        assert uart_send_nextion(m, ffi, first) == 1
        assert uart_send_nextion(m, ffi, second) == 1
        # Buffer full; cannot wrap while head still occupies the front
        assert uart_send_nextion(m, ffi, wrapped) == 0

        uart_consume_nextion(m)
        assert uart_send_nextion(m, ffi, wrapped) == 1
        assert uart_send_nextion(m, ffi, follow) == 1

        # Middle free space is exhausted (50+20 used of the freed 80)
        assert uart_send_nextion(m, ffi, bytes([0x55] * (15 - eot))) == 0


class TestUARTServiceTXCapacity(TestParent):
    def test_max_descriptors_drops(self):
        frame = bytes([0xAA, 0xBB])
        count = m.UART_SERVICE_TX_MSG_DESCRIPTORS_COUNT
        channel = m.UART_CHANNEL_SERVICE

        for i in range(count):
            assert uart_write_message(m, ffi, channel, frame) == 1, f"enqueue {i} should succeed"

        assert uart_write_message(m, ffi, channel, frame) == 0

    def test_max_buffer_drops_with_descriptors_free(self):
        payload_len = 10
        payload = bytes([0x41 + (i % 26) for i in range(payload_len)])
        framed_len = payload_len + m.UART_SERVICE_TERMINATOR_LEN
        expected_ok = m.UART_SERVICE_TX_BUFFER_SIZE // framed_len
        channel = m.UART_CHANNEL_SERVICE

        assert expected_ok < m.UART_SERVICE_TX_MSG_DESCRIPTORS_COUNT

        for i in range(expected_ok):
            assert uart_write_message(m, ffi, channel, payload) == 1, f"enqueue {i} should succeed"

        assert uart_write_message(m, ffi, channel, payload) == 0

        uart_consume_message(m, channel)
        assert uart_write_message(m, ffi, channel, payload) == 1

    def test_terminator_appended(self):
        channel = m.UART_CHANNEL_SERVICE
        m.SERIAL_SERVICE_OUT_status = m.SERIAL_OUT_STATUS_IDLE
        payload = bytes([0xAA, 0xBB])
        assert uart_write_message(m, ffi, channel, payload) == 1
        assert m.service_tx_message_length == len(payload) + m.UART_SERVICE_TERMINATOR_LEN
        assert bytes(ffi.unpack(m.service_tx_message, m.service_tx_message_length)) == payload + b"\r"
        assert read_service_output(m, ffi) == [payload]


class TestUARTServiceTXWrap(TestParent):
    def test_wrap_to_front_after_consume(self):
        channel = m.UART_CHANNEL_SERVICE
        eot = m.UART_SERVICE_TERMINATOR_LEN
        # Framed sizes: 18 + 14 = 32
        first = bytes([0x11] * (18 - eot))
        second = bytes([0x22] * (14 - eot))
        wrapped = bytes([0x33] * (10 - eot))
        follow = bytes([0x44] * (6 - eot))

        assert uart_write_message(m, ffi, channel, first) == 1
        assert uart_write_message(m, ffi, channel, second) == 1
        assert uart_write_message(m, ffi, channel, wrapped) == 0

        uart_consume_message(m, channel)
        assert uart_write_message(m, ffi, channel, wrapped) == 1
        assert uart_write_message(m, ffi, channel, follow) == 1
        assert uart_write_message(m, ffi, channel, bytes([0x55] * (4 - eot))) == 0


class TestUARTNextionRX(TestParent):
    channel = m.UART_CHANNEL_NEXTION
    terminator = b"\xff\xff\xff"

    def feed_message(self, payload):
        uart_feed(self.channel, payload + self.terminator)

    def test_message_is_published_without_terminator(self):
        uart_feed(self.channel, b"abc\xff\xff")
        assert uart_get(self.channel) is None

        uart_feed(self.channel, b"\xff")
        _, payload = uart_get(self.channel)
        assert payload == b"abc"

    def test_message_keeps_receive_timestamp(self):
        m.SYSTEM_event_timer = 3
        m.TCNT2 = 7
        self.feed_message(b"timestamp")

        message = ffi.new("uint8_t **")
        timestamp = ffi.new("SYSTEM_cycle_timestamp_t *")
        length = m.UART_get_next_message(self.channel, message, timestamp)

        assert bytes(ffi.unpack(message[0], length)) == b"timestamp"
        assert timestamp[0] == 3 * 16 + 7

    def test_post_irq_core_dispatches_touch_message(self):
        touch_message = bytes([
            m.NEXTION_MESSAGEHEADER_TOUCHINPUT,
            0,
            m.INPUT_COMPONENT_NONE,
            m.INPUT_KEYSTATUS_PRESSED,
        ])
        self.feed_message(touch_message)

        m.post_irq_core()

        assert m.INPUT_keystatus[m.INPUT_KEY_ENTER] == m.INPUT_KEYSTATUS_PRESSED
        assert uart_get(self.channel) is None

    def test_non_consecutive_ff_does_not_terminate(self):
        stream = b"a\xff\xffb\xff\xffc\xff\xff\xff"
        uart_feed(self.channel, stream)
        _, payload = uart_get(self.channel)
        assert payload == b"a\xff\xffb\xff\xffc"

    def test_fifo_and_deferred_release(self):
        messages = [b"one", b"two", b"three", b"four"]
        for payload in messages:
            self.feed_message(payload)

        first_ptr, first = uart_get(self.channel)
        assert first == messages[0]
        assert bytes(ffi.unpack(first_ptr, len(first))) == first

        _, second = uart_get(self.channel)
        assert second == messages[1]

        self.feed_message(b"five")
        assert [uart_get(self.channel)[1] for _ in range(3)] == [
            messages[2],
            messages[3],
            b"five",
        ]
        assert uart_get(self.channel) is None

    def test_descriptor_exhaustion_drops_whole_frame_then_recovers(self):
        count = m.UART_NEXTION_RX_MSG_DESCRIPTORS_COUNT
        for value in range(count):
            self.feed_message(bytes([ord("a") + value]))

        self.feed_message(b"dropped")
        _, first = uart_get(self.channel)
        assert first == b"a"
        _, second = uart_get(self.channel)
        assert second == b"b"

        self.feed_message(b"accepted")
        remaining = [uart_get(self.channel)[1] for _ in range(count - 2)]
        assert remaining == [b"c", b"d"]
        _, accepted = uart_get(self.channel)
        assert accepted == b"accepted"
        assert uart_get(self.channel) is None

    def test_oversized_frame_drops_until_terminator_then_recovers(self):
        oversized = bytes([0x41] * m.NEXTION_RX_BUFFER_SIZE)
        uart_feed(self.channel, oversized + self.terminator)
        assert uart_get(self.channel) is None

        self.feed_message(b"ok")
        _, payload = uart_get(self.channel)
        assert payload == b"ok"

    def test_wraps_to_larger_contiguous_region(self):
        first = bytes([0x11] * 17)
        second = bytes([0x22] * 6)
        self.feed_message(first)
        self.feed_message(second)

        assert uart_get(self.channel)[1] == first
        assert uart_get(self.channel)[1] == second

        wrapped = bytes([0x33] * 12)
        self.feed_message(wrapped)
        _, payload = uart_get(self.channel)
        assert payload == wrapped

    def test_empty_message_and_init_are_empty(self):
        uart_feed(self.channel, self.terminator)
        assert uart_get(self.channel) is None

        self.feed_message(b"pending")
        m.UART_init()
        assert uart_get(self.channel) is None


class TestUARTServiceRX(TestParent):
    channel = m.UART_CHANNEL_SERVICE

    def test_cr_terminates_and_is_stripped(self):
        uart_feed(self.channel, b"service\r")
        _, payload = uart_get(self.channel)
        assert payload == b"service"

    def test_single_descriptor_requires_release_and_recovers(self):
        uart_feed(self.channel, b"first\rsecond\r")
        _, payload = uart_get(self.channel)
        assert payload == b"first"
        assert uart_get(self.channel) is None

        uart_feed(self.channel, b"third\r")
        _, payload = uart_get(self.channel)
        assert payload == b"third"

    def test_empty_message_is_discarded(self):
        uart_feed(self.channel, b"\r")
        assert uart_get(self.channel) is None
