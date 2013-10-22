## Drink Dispensing Coordinator


#### Dispense Codes
D -> Dispense with the following ingredients
F -> Finished
Y -> Yes/OK/Continue
N -> NO/Stop
T -> End Line
Z -> Finished dispensing the current drink


#### Sample Dispense Code
```` bash
D3000T3000TF
````

Examples:

	BBB: Tries to send a drink to dispense
	CB: Available to dispnse
	Result: CB succesfully dispenses

	BB == BeagleBone Black
	CB == Control Board (Atmega)

		Direction  Value Sent
		BB -> CB 	D
		CB -> BB 	Y
		BB -> CB 	<ing0>T
		CB -> BB 	Y
		BB -> CB 	<ing1>T
		CB -> BB 	Y
		.
		.
		.
		BB -> CB 	<ingN>T
		CB -> BB 	Y
		BB -> CB 	F
		CB -> BB 	Y
		[Dispense Drink Here]
		CB -> BB 	Z
		BB -> CB 	Y