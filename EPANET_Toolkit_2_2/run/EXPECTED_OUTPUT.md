# Expected Results
## ./evalpattern vanZyl.inp schedules0
	Network: vanZyl.inp: 18 links  13 junctions  3 pumps and  2 tanks
	
	warning: 'schedules0' contains more patterns than pumps in 'vanZyl.inp'
	Pump: (Switches)[MinIdleTime]: Schedule
	pmp1: (       1)[      61200]: 111111100000000000000000
	pmp2: (       1)[      32400]: 111111100000000011111111
	pmp6: (       1)[      39600]: 111111100000000000111111
	-------------------------------------------------------------
	Total switches = 3, Min Idle Time = 32400
	
	 Tank: Head(0) - Head = dHead : V(0) - Volume = dVolume
	   t6: 94.50 - 91.49 = +3.01 : 2984.51 - 2038.29 = +946.22
	   t5: 84.50 - 83.73 = +0.77 : 2208.93 - 1831.06 = +377.87
	
	Total demand:  12776402.00
	Total inflow: -11452298.00
	             =  1324104.0000
	Total tanks:   1324.0947 m^3
	Difference = -9.27 
	Total leakage: 0.00
	
	Total energy cost:       274.34

## ./evalpattern vanZyl.inp schedules1
	Network: vanZyl.inp: 18 links  13 junctions  3 pumps and  2 tanks
	
	Pump: (Switches)[MinIdleTime]: Schedule
	pmp1: (       1)[      61200]: 000000000000000001111111
	pmp2: (       1)[      14400]: 000111111111111111111110
	pmp6: (       1)[      39600]: 000000000001111111111111
	-------------------------------------------------------------
	Total switches = 3, Min Idle Time = 14400
	
	 Tank: Head(0) - Head = dHead : V(0) - Volume = dVolume
	   t6: 94.50 - 90.44 = +4.06 : 2984.51 - 1710.11 = +1274.41
	   t5: 84.50 - 84.90 = -0.40 : 2208.93 - 2406.13 = -197.20
	
	Total demand:  12776635.00
	Total inflow: -11783168.00
	             =  993467.0000
	Total tanks:   1077.2095 m^3
	Difference = 83742.47 
	Total leakage: 0.00
	
	Total energy cost:       431.23

## ./evalpattern vanZyl.inp schedules2
	Network: vanZyl.inp: 18 links  13 junctions  3 pumps and  2 tanks
	
	Pump: (Switches)[MinIdleTime]: Schedule
	pmp1: (      12)[       3600]: 101010101010101010101010
	pmp2: (      12)[       3600]: 010101010101010101010101
	pmp6: (       1)[      36000]: 100000000001111111111111
	-------------------------------------------------------------
	Total switches = 25, Min Idle Time = 3600
	
	 Tank: Head(0) - Head = dHead : V(0) - Volume = dVolume
	   t6: 94.50 - 92.60 = +1.90 : 2984.51 - 2386.66 = +597.85
	   t5: 84.50 - 84.97 = -0.47 : 2208.93 - 2438.59 = -229.66
	
	Total demand:  12776402.00
	Total inflow: -12408115.00
	             =  368287.0000
	Total tanks:    368.1897 m^3
	Difference = -97.30 
	Total leakage: 0.00
	
	Total energy cost:       442.51