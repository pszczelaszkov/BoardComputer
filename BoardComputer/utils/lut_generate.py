import argparse
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator


def parse_args():
    parser = argparse.ArgumentParser(description="Generate NTC ADC->Temperature lookup table")
    parser.add_argument("--r25", type=int, default=2200.0, help="NTC resistance at 25C (ohms)")
    parser.add_argument("--beta", type=int, default=3950.0, help="Beta coefficient")
    parser.add_argument("--rseries", type=int, default=2200.0, help="Series resistor value (ohms)")
    parser.add_argument("--clamp-min", type=int, default=-32767, help="Minimum allowed temperature (°C)")
    parser.add_argument("--clamp-max", type=int, default=32768, help="Maximum allowed temperature (°C)")
    parser.add_argument("--compression-level", type=int, default=1, help="Levels of compression 1-Default 1024 LUT, 2-Fit Lut in 512 bytes, 3-Fit in 256 bytes")
    parser.add_argument("--offset", type=int, default=0, help="If LUT is in the form of 8-bit values, define offset. Must match offset defined in C file.")
    return parser.parse_args()


def generate_table(r25, beta, rseries, clamp_min=None, clamp_max=None):
    t0 = 25.0 + 273.15
    temperature_table = np.full(1024, np.nan)

    # adc == 0 and adc == 1023 are always invalid (saturated readings)
    for adc in range(1, 1023):

        # NTC to GND:
        #
        # Vout = Vcc * Rntc / (Rseries + Rntc)
        #
        # adc = 1023 * Vout/Vcc
        #
        # Rntc = Rseries * adc / (1023 - adc)

        r_ntc = rseries * adc / (1023.0 - adc)

        # B-parameter equation
        #
        # 1/T = 1/T0 + (1/B) * ln(R/R25)

        temp_k = 1.0 / (
                (1.0 / t0)
                + (1.0 / beta) * np.log(r_ntc / r25)
        )

        temp_c = temp_k - 273.15
        temp_c = max(clamp_min, min(temp_c, clamp_max))
        temperature_table[adc] = temp_c

    valid_start = 1
    valid_end = 1022  # inclusive

    return temperature_table, valid_start, valid_end

def main():
    args = parse_args()

    temperature_table, valid_start, valid_end = generate_table(
        r25=args.r25,
        beta=args.beta,
        rseries=args.rseries,
        clamp_min=args.clamp_min,
        clamp_max=args.clamp_max,
    )

    compression_step = max(1, min(args.compression_level, 2))

    final_table = temperature_table[valid_start:valid_end + 1]
    valid_mask = ~np.isnan(final_table)
    sensitivity = np.full(final_table.shape, np.nan)
    valid_values = final_table[valid_mask]
    if valid_values.size > 1:
        sensitivity[valid_mask] = np.abs(np.gradient(valid_values))

    fig, ax = plt.subplots(2, 1, figsize=(10, 10))

    ax[0].plot(temperature_table)
    ax[0].set_title("ADC / Temp")
    ax[0].set_xlabel("ADC")
    ax[0].set_ylabel("Temp[°C]")
    ax[0].grid(True, which='major')
    ax[0].xaxis.set_minor_locator(AutoMinorLocator(10))
    ax[0].yaxis.set_minor_locator(AutoMinorLocator(10))
    ax[0].grid(True, which='minor', alpha=0.3)

    ax[1].plot(range(valid_start, valid_end + 1), sensitivity)
    ax[1].set_title("°C / LSB")
    ax[1].set_xlabel("ADC")
    ax[1].set_ylabel("°C / ADC step")
    ax[1].grid(True, which='major')
    ax[1].xaxis.set_minor_locator(AutoMinorLocator(10))
    ax[1].yaxis.set_minor_locator(AutoMinorLocator(10))
    ax[1].grid(True, which='minor', alpha=0.3)

    plt.tight_layout()
    plt.savefig("ntc_analysis.png", dpi=300)

    with open("ntc_lut.txt", "w") as f:
        f.write(f"[PROGRAMDATA_ADC_LUT_NTC_{args.r25}R25_{args.rseries}RS_{args.beta}B]{{")
        for i, val in enumerate(temperature_table[::compression_step]):
            val = int(val)+args.offset if not np.isnan(val) else 0
            f.write(f"{val},")
        f.write("}\n")

    print("LUT written to ntc_lut.txt")
    print("Analysis plot written to ntc_analysis.png")


if __name__ == "__main__":
    main()
