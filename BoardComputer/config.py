'''
    That file is truth source for configuration values.
    It also generates ready to be copied string to nextion rfp config event.
    Goal is to help maintain conformity. 
'''


MAXVALUE = 32768
MINVALUE = -32767

DESCRIPTION_MAX_LENGTH=96
NAME_MAX_LENGTH=16
CATEGORY_MAX_LENGTH=10

ENTRY_VALIDATORS = {
    "ENTRY_VALIDATOR_NONE": (None, None),
    "ENTRY_VALIDATOR_BOOLEAN": (0, 1), # True/False validator
    "ENTRY_VALIDATOR_ENUM_2": (0, 2),
    "ENTRY_VALIDATOR_ENUM_3": (0, 3),
    "ENTRY_VALIDATOR_ENUM_4": (0, 4),
    "ENTRY_VALIDATOR_ENUM_5": (0, 5),
    "ENTRY_VALIDATOR_ENUM_6": (0, 6),
    "ENTRY_VALIDATOR_ENUM_7": (0, 7),
    "ENTRY_VALIDATOR_ENUM_8": (0, 8),
    "ENTRY_VALIDATOR_ENUM_9": (0, 9),
    "ENTRY_VALIDATOR_POSITIVE_EXCL_0": (1, MAXVALUE),# Check if maxvalue >= value > 0
    "ENTRY_VALIDATOR_POSITIVE_INCL_0": (0, MAXVALUE), # Check if maxvalue >= value >=0
    "ENTRY_VALIDATOR_NEGATIVE_EXCL_0": (MINVALUE, -1), # Check if minvalue <= value < 0
    "ENTRY_VALIDATOR_NEGATIVE_INCL_0": (MINVALUE, 0), # Check if minvalue <= value <=0
    "ENTRY_VALIDATOR_POSITIVE_4DIGIT_EXCL_0": (1, 9999), # Check if 9999 >= value > 0
    "ENTRY_VALIDATOR_POSITIVE_4DIGIT_INCL_0": (0, 9999), # Check if 9999 >= value >=0
    "ENTRY_VALIDATOR_NEGATIVE_4DIGIT_EXCL_0": (-9999, -1), # Check if -9999 <= value < 0
    "ENTRY_VALIDATOR_NEGATIVE_4DIGIT_INCL_0": (-9999, 0), # Check if -9999 <= value <= 0
    "ENTRY_VALIDATOR_ALL_4DIGIT": (-9999,9999), # Check if -9999 <= value <= 9999
    "ENTRY_VALIDATOR_ALL_3DIGIT": (-999,999), # Check if -999 <= value <= 999
    "ENTRY_VALIDATOR_ALL_2DIGIT": (-99,99), # Check if -99 <= value <= 99
    "ENTRY_VALIDATOR_PERCENT": (0, 100), # Check if 0 <= value <= 100
    "ENTRY_VALIDATOR_CALIBRATION_VALUE": (-100,100), # Check if -100 <= value <= 100 */
}

config = [
    {"category": "SYSTEM", "name":"FACTORY_RESET", "desc": "When True, device will reboot and reset settings.", "validator": "ENTRY_VALIDATOR_BOOLEAN", "size": 1, "default": 0},
    {"category": "SYSTEM", "name":"ALWAYS_ON", "desc": "If enabled, ignore ignition signal.", "validator": "ENTRY_VALIDATOR_BOOLEAN", "size": 1, "default": 1},
    {"category": "SYSTEM", "name":"BEEP_ON_CLICK", "desc": "Enable if audible feedback is needed.", "validator": "ENTRY_VALIDATOR_BOOLEAN", "size": 1, "default": 0},
    {"category": "SYSTEM", "name":"BRIGHTNESS", "desc": "Display brightness.", "validator": "ENTRY_VALIDATOR_PERCENT", "size": 1, "default": 100},
    {"category": "SENSORS", "name":"SIGNAL_PER_100M", "desc": "Enter how many signals wheel need for 100 Meter.", "validator": "ENTRY_VALIDATOR_POSITIVE_4DIGIT_EXCL_0", "size": 2, "default": 1},
    {"category": "SENSORS", "name":"INJECTORS_CCM", "desc": "Enter injectors CCM value, remember to multiple if injectors fire in pairs.", "validator": "ENTRY_VALIDATOR_POSITIVE_4DIGIT_EXCL_0", "size": 2, "default": 1},

    {"category": "SENSORS", "name":"OILTEMP_CAL", "desc": "Oil temperature ADC calibration offset.", "validator": "ENTRY_VALIDATOR_CALIBRATION_VALUE", "size": 1, "default": 0},
    {"category": "SENSORS", "name":"INTAKETEMP_CAL", "desc": "Intake temperature ADC calibration offset.", "validator": "ENTRY_VALIDATOR_CALIBRATION_VALUE", "size": 1, "default": 0},
    {"category": "SENSORS", "name":"OUTTEMP_CAL", "desc": "Outside temperature ADC calibration offset.", "validator": "ENTRY_VALIDATOR_CALIBRATION_VALUE", "size": 1, "default": 0},

    {"category": "SENSORS", "name":"MAP_MIN", "desc": "Manifold pressure sensor minimum value (kPa).", "validator": "ENTRY_VALIDATOR_ALL_3DIGIT", "size": 2, "default": 0},
    {"category": "SENSORS", "name":"MAP_MAX", "desc": "Manifold pressure sensor maximum value (kPa).", "validator": "ENTRY_VALIDATOR_ALL_3DIGIT", "size": 2, "default": 500},

    {"category": "SENSORS", "name":"FRP_MIN", "desc": "Fuel rail pressure sensor minimum value (kPa).", "validator": "ENTRY_VALIDATOR_ALL_3DIGIT", "size": 2, "default": 0},
    {"category": "SENSORS", "name":"FRP_MAX", "desc": "Fuel rail pressure sensor maximum value (kPa).", "validator": "ENTRY_VALIDATOR_ALL_3DIGIT", "size": 2, "default": 500},

    {"category": "SENSORS", "name":"TANK_MIN", "desc": "Fuel tank sensor minimum (Liter).", "validator": "ENTRY_VALIDATOR_PERCENT", "size": 1, "default": 0},
    {"category": "SENSORS", "name":"TANK_MAX", "desc": "Fuel tank sensor maximum (Liter).", "validator": "ENTRY_VALIDATOR_PERCENT", "size": 1, "default": 0},

    {"category": "SENSORS", "name":"EGT_INTERNAL", "desc": "Use internal sensor for EGT. If disabled, use ADC channel(1-1022 C).", "validator": "ENTRY_VALIDATOR_BOOLEAN", "size": 1, "default": 1},

    {"category": "BOARD", "name":"DELTA_THRESHOLD", "desc": "Set pressure threshold (Bar) for Fuel to Manifold pressure delta indicator.", "validator": "ENTRY_VALIDATOR_ENUM_3", "size": 1, "default": 0},
]


def generate_boilerplate():
    return """\
//Define slide limits
if(max.val<{MAXVALUE})
{{
  bar.minval=min.val+{BELOW_MAXVALUE}
  bar.maxval=max.val+{BELOW_MAXVALUE}
  bar.val=vlh.val+{BELOW_MAXVALUE}
  bar.pic2={active_slider_pic}
}}else
{{
  bar.minval=0
  bar.maxval=0
  bar.val=0
  bar.pic2={unactive_slider_pic}
}}
if(-1==res.val)
{{
  inf.txt="Value is too small"
}}else if(1==res.val)
{{
  inf.txt="Value is too big"
}}
rfp.en=0
res.val=0"""

def generate_if_string():
    result_lines = []
    for i, entry in enumerate(config):
        prefix = "if" if i == 0 else "else if"
        description = entry["desc"]
        category = entry["category"]
        name = entry["name"]
        if len(description) > DESCRIPTION_MAX_LENGTH:
            print(f"{category}_{name} has description longer than {DESCRIPTION_MAX_LENGTH}!")
            return
        if len(category) > CATEGORY_MAX_LENGTH:
            print(f"{category}_{name} has category longer than {CATEGORY_MAX_LENGTH}!")
            return
        if len(name) > NAME_MAX_LENGTH:
            print(f"{category}_{name} has name longer than {NAME_MAX_LENGTH}!")
            return

        block = (
            f"{prefix}({i}==ptr.val)\n"
            "{\n"
            f'  dsc.txt="{entry["desc"]}"\n'
            f'  cat.txt="{entry["category"]}"\n'
            f'  nam.txt="{entry["name"]}"\n'
            "}"
        )
        result_lines.append(block)

    # Slider setup and result readout:
    if_lines = "\n".join(result_lines)
    boilerplate_lines = "".join(generate_boilerplate().format(MAXVALUE=MAXVALUE,BELOW_MAXVALUE=MAXVALUE-1,active_slider_pic=31,unactive_slider_pic=30))
    with open("config_rfpcontent.txt", "w") as file:
        file.write(f"//Generated content by config.py\n{if_lines}\n{boilerplate_lines}\n//End of generated content")

    print("Succesfully created rfp content!")
    print("Copy config_rfpcontent.txt to nextion config page rfp event code.")

if __name__ == "__main__":
    generate_if_string()
