[WIP] sysfs entries for axp20x mfd driver
===========
```bash
➜  ~  % ls /sys/power/axp_pmu/{ac,vbus,battery,charger,pmu,control}
/sys/power/axp_pmu/ac:
amperage  connected  used  voltage

/sys/power/axp_pmu/battery:
amperage  capacity  charge  charging  connected  power  ts_voltage  voltage

/sys/power/axp_pmu/charger:
amperage  cell_activation  charging  low_power

/sys/power/axp_pmu/control:
reset_charge_counter  set_vbus_direct_mode

/sys/power/axp_pmu/pmu:
cold_boot  overheat  temp  voltage

/sys/power/axp_pmu/vbus:
amperage  connected  strong  used  voltage
➜  ~  %
```

Properties table
===========

| Subsystem | Attribute  | Type    | Unit | AXP Reg  | Notes    |
| --------- | ---------- | ------- | ---- | -------- | -------- |
| ac        | voltage    | integer | µV   | 56h:57h  |          |
| ac        | amperage   | integer | µA   | 58h:59h  |          |
| ac        | connected  | boolean | -    | 00h[7]   |          |
| ac        | used       | boolean | -    | 00h[6]   |          |
| vbus      | voltage    | integer | µV   | 5ah:5bh  |          |
| vbus      | amperage   | integer | µA   | 5ch:5dh  |          |
| vbus      | connected  | boolean | -    | 00h[5]   |          |
| vbus      | used       | boolean | -    | 00h[4]   |          |
| vbus      | strong     | boolean | -    | 00h[3]   |          |
| battery   | voltage    | integer | µV   | 78h:79h  |          |
| battery   | amperage   | integer | µA   | 7ch:7dh  | Discharge current |
| battery   | ts_voltage | integer | µV   | 62h:63h  |          |
| battery   | power      | integer | µW   | 70h:72h  |          |
| battery   | capacity   | integer | %    | b9h[6:0] |          |
| battery   | charge     | integer | µAh  | b0h:b7h  |          |
| battery   | connected  | boolean | -    | 01h[5]   |          |
| battery   | charging   | boolean | -    | 00h[7]   | Direction of current |
| pmu       | temp       | integer | m°C  | 5eh:5fh  |          |
| pmu       | voltage    | integer | µV   | 7eh:7fh  |          |
| pmu       | cold_boot  | boolean | -    | 00h[0]   |          |
| pmu       | overheat   | boolean | -    | 01h[7]   |          |
| charger   | amperage   | integer | µA   | 7ah:7bh  | Charge current |
| charger   | charging   | boolean | -    | 01h[6]   |          |
| charger   | cell_activation | boolean | - | 01h[3] | Mode for charging deeply discharged cell |
| charger   | low_power | boolean | - | 01h[2] | Not enough input power |
| control   | set_vbus_direct_mode | boolean | - |    |          |
| control   | reset_coulomb_counter | boolean |       |          |