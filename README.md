[WIP] sysfs entries for axp20x mfd driver
===========

For mainline kernel 4.4+

Main purpose of this patch is creating sysfs interface to get information from AXP202/209 PMU until proper power driver is implemented in mainline kernel.

```bash
➜  ~  % ls /sys/power/axp_pmu/{ac,vbus,battery,charger,pmu,control,ocv_curve}
/sys/power/axp_pmu/ocv_curve

/sys/power/axp_pmu/ac:
amperage  connected  used  voltage

/sys/power/axp_pmu/battery:
amperage  capacity  charge  charging  connected  power  ts_voltage  voltage

/sys/power/axp_pmu/charger:
amperage  cell_activation  charging  low_power

/sys/power/axp_pmu/control:
battery_rdc         disable_fuel_gauge    set_vbus_direct_mode
charge_rtc_battery  reset_charge_counter

/sys/power/axp_pmu/pmu:
overheat  temp  voltage

/sys/power/axp_pmu/vbus:
amperage  connected  strong  used  voltage
➜  ~  %
```

Properties table
===========

| Subsystem | Attribute             | Type    | Unit | AXP Reg  | Notes    |
| --------- | --------------------- | ------- | ---- | -------- | -------- |
| ac        | voltage               | integer | µV   | 56h:57h  |          |
| ac        | amperage              | integer | µA   | 58h:59h  |          |
| ac        | connected             | boolean | -    | 00h[7]   |          |
| ac        | used                  | boolean | -    | 00h[6]   |          |
| vbus      | voltage               | integer | µV   | 5Ah:5Bh  |          |
| vbus      | amperage              | integer | µA   | 5Ch:5Dh  |          |
| vbus      | connected             | boolean | -    | 00h[5]   |          |
| vbus      | used                  | boolean | -    | 00h[4]   |          |
| vbus      | strong                | boolean | -    | 00h[3]   |          |
| battery   | voltage               | integer | µV   | 78h:79h  |          |
| battery   | amperage              | integer | µA   | 7Ch:7Dh  | Discharge current |
| battery   | ts_voltage            | integer | µV   | 62h:63h  |          |
| battery   | power                 | integer | µW   | 70h:72h  |          |
| battery   | capacity              | integer | %    | B9h[6:0] |          |
| battery   | charge                | integer | µAh  | B0h:B7h  |          |
| battery   | connected             | boolean | -    | 01h[5]   |          |
| battery   | charging              | boolean | -    | 00h[7]   | Direction of current |
| pmu       | temp                  | integer | m°C  | 5Eh:5Fh  |          |
| pmu       | voltage               | integer | µV   | 7Eh:7Fh  |          |
| pmu       | overheat              | boolean | -    | 01h[7]   |          |
| charger   | amperage              | integer | µA   | 7Ah:7Bh  | Charge current |
| charger   | charging              | boolean | -    | 01h[6]   |          |
| charger   | cell_activation       | boolean | -    | 01h[3]   | Mode for charging deeply discharged cell |
| charger   | low_power             | boolean | -    | 01h[2]   | Not enough input power |
| control   | set_vbus_direct_mode  | boolean | -    | 30h[6]   |          |
| control   | reset_charge_counter  | boolean | -    | B8h[5]   |          |
| control   | charge_rtc_battery    | boolean | -    | 35h[7]   |          |
| control   | disable_fuel_gauge    | boolean | -    | B9h[7]   |          |
| control   | battery_rdc           | integer | µΩ   | BAh:BBh  |          |
| -         | ocv_curve             | byte[]  | %    | C0h:CFh  |          |


Manual battery calibration tool
===========

```
Usage: axp20x-calibrate-battery [-h | -p | -e | -l <file> | -s <file>]
Arguments:
-h:             Show this help message and exit
-p:             Print configuration
-e:             Edit configuration
-s <file>:      Save OCV table to <file>
-l <file>:      Load and apply OCV table from <file>
```

Editing (-e) and loading (-l) requires root privileges
