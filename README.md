tangy
=====

tangy is picture generator.
It is inspired by pic (roff pre-processor).
Using the language, you can draw picture in EPS (Encapsuled Postscript) file.
For example, following script makes DB, client and their communication
through Internet.

    drum "DB"
    arrow
    cloud "Internet"
    arrow
    circle "client"

<img src="sample0.png">

You can change color, hatching and line types.

	drum "A"
	arrow 
	box "B"
	arrow linetype dashed
	circle "C" fillhatch slashed fillcolor 3
	arrow linetype zigzag
	ellipse "D" fillhatch crossed fillcolor 2
	arrow linetype waved

<img src="sample1.png">

Tangys supports "alias" to draw simular shapes.

	alias ent "box fillcolor 15 fillhatch solid"
	box width 0 height 2u linecolor -1
	ent "preamble"			:NOC "8"
	ent "ethernet" "header"		:NOC "14"
	ent "IP" "header"		:NOC "20+"
	ent "TCP" "header"		:NOC "20+"
	ent "data" width 2u vwslit	:NOC "6..1460"
	ent "FCS"  width .8u		:NOC "4"
	ent "IFG" 			:NOC "12"

<img src="sample3.png">

Tangy accepts ASCII, Japanese(UTF-8) and mix of them.

	box "ASCII日本語" width 2u
	move
	box "|sanserif|ASCII日本語" width 2u

<img src="sample2.png">

Thanks.

