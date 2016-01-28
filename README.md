[WIP] sysfs entries for axp20x mfd driver
===========

For mainline kernel 4.4+

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