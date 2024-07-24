---
layout: post
title: Digital Video Commander Cable Build Instructions
---

This documentation will explain how to build your own Digital Video Commander Cable. For usage and testing of cables see the [relevant documentation](cableTesting.html).

__These instructions involve the use of sharp tools such as utility knives - follow at your own risk and observe all necessary safety precautions. If you are uncomfortable handling these tools do not proceed.__ 

There are two versions of the cable. The first is slightly more expensive to assemble as it requires additional components, but the result is a more enclosed cable. The second is less involved with all of wiring exposed. These instructions cover the first version. Corresponding video training of how to build Version 2 coming soon.

For details on how to build Version 1, please see the corresponding training video and documentation.


## Supplies
To build Version 1 of the Digital Video Commander cable, you will need the following supplies. 
Note that one will have 9 pins and one will have 9 holes. We shall refer to these as innies and outies for all related Digital Video Commander documentation.

* 9-pin cable (28 AWG 9/C CMP Plenum Rated Double Shielded Security RS232 Data Cable)
* Wire strippers (30-20 AWG)
* Micro wire cutter
* #2 Phillips screwdriver 
* Box cutter
* A protective surface for cutting on (a cutting board or scrap of cardboard both work well)
* USB to RS422 RS485 Serial Port Converter Adapter Cable with FTDI Chip
* 2 Connector DB9 RS232 D-SUB Serial Adapters (Note that one will have 9 pins and one will have 9 holes. We shall refer to these as innies and outies.)

  - 1 Connector DB9 RS232 D-SUB Serial Adapters (Innie Adapter)
  - 1 Connector DB9 RS232 D-SUB Serial Adapters (Outie Adapter)
* DB9 Outie (Male) to Innie (Female) RS232 Extension Serial Cable
* Each of the innie and outie adapters comes with the following accessories:
  - 2 long bolts
  - 1 shell case
  - 1 DB9 terminal
  - 3 rubber wire grommets in 5mm, 6mm, and 8mm. We have found that the 6mm works the best for building this cable.
  - 2 short screws
  - 1 cable clamp
  - flat head assembly screwdriver

For purchasing recommendations please see our complete documentation on the Digital Video Commander website.

## Wiring Overview
See below for diagrams of MIPoPS preferred wiring colors and technique (click for full size). Both the innie and outie adapters are encased in a removable plastic shell. This separate part is the terminal. The numbers along the sides of each refer to the pin that will house the corresponding colored wire on each side for each adapter. To view or download a copy of this diagram and glossary of terms, please see the documentation section of the Digital Video Commander

<a href="{{ site.baseurl }}/images/DigitalVideoCommander_Cable_WiringMap_V2_Page1.png"><img alt="Wiring map page one" src="{{ site.baseurl }}/images/DigitalVideoCommander_Cable_WiringMap_V2_Page1.png"></a>

<a href="{{ site.baseurl }}/images/DigitalVideoCommander_Cable_WiringMap_V2_Page2.png"><img alt="Wiring map page two" src="{{ site.baseurl }}/images/DigitalVideoCommander_Cable_WiringMap_V2_Page2.png"></a>

## Assembly Instructions

__Step 1:__ With the wire cutters, cut the cable to the desired length. You can always get a longer 9-pin RS232 Extension Serial Cable to achieve the desired length to establish a connection between your deck and your computer. 

__Step 2:__ Slide a rubber wire grommets onto each end of the cable, with the flat end facing the end of the cable. Note that the recommended wiring adapters include three sizes of stopper (5mm, 6mm, 7mm), so use the one that fits the diameter of the cable best. The 6mm fits the best. 

__Step 3:__  Place the trimmed cable on a cutting surface. With the wire laying in a horizontal line, use the box cutter to score a vertical mark along the outer casing 3.5cm down from the end of the cable (about the length of the terminal shell). Slice through the casing from the cut you just made to the end of the cable. Pull the casing the rest of the way off or trim it with the wire cutter. Cut off the shielding using the wire cutter to expose the wires inside. Repeat this step for the other side, but make your first score 0.5cm further down the wire. Trim all of the wires except the yellow and orange wire by 0.5cm. This will make it easier for them to reach their connection. Reserve this longer end for wiring the outie terminal.

__Step 4:__ Using the 0.4 millimeter trimming position on the wire strippers, trim the insulation off of all 9 wires from within the cable to reveal about 0.5cm of the copper wire within. Repeat this step for the wires on the other side. On one end of the cable (to be attached to the outie end), leave the yellow and orange wires longer than the others by about a half centimeter. This will make it easier for them to reach their connection. Reserve this longer end for wiring the outie terminal.

__Step 5:__ Remove the two terminals from their plastic cases. If there is a grommet there, remove and discard it (this is usually the smallest size 5mm grommet and is too small for the 9-pin cable). First, let's wire up the innie terminal.

__Step 6:__ On the innie terminal, loosen the wiring screws for pins 1, 2, 3, 4, 5, and “Gound” using the assembly flathead screwdriver. Each should open all the way to expose a pin (or “room”) within the terminal. The screw will begin to feel tight once the pin is all the way open.

__Step 7:__ Our instructions are based on using the red, orange, yellow, green, and blue wires for the main connections. Using the wiring diagram as a guide, press the exposed copper wire of the 5 wires into the corresponding numbered rooms (or pins), one by one, screwing the pin shut after inserting each wire. Twist the exposed copper of the gray, black, white, and brown wires together and insert them into the “Ground” pin/room and screw the pin shut. For all 9 wires, ensure the exposed copper is completely inside the terminal and cannot come into contact with any other wires.

__Step 8:__ On the outie terminal, unscrew pins 1, 2, 3, 7, 8, and “Ground.” Using the wiring diagram as a guide, press the exposed copper wire of the 5 wires into the corresponding numbered rooms (or pins), one by one, screwing the pin shut after inserting each wire. Twist the exposed copper of the gray, black, white, and brown wires together and insert them into the “Ground” pin/room and screw the pin shut.

__Step 9:__ Fit the innie terminal back into the case, matching the notches in the terminal to the shape of the case. Fit the flat end of the stopper into the case and leave about 0.75cm of the cable protruding beyond the stopper and into the case.

__Step 10:__ Add the cable brace to hold the cable inside the case using the two provided screws to secure the brace on the outer part of the case and tighten it. Close the case.

__Step 11:__ Repeat steps 9 and 10 for the outie terminal.

__Step 12:__ Thread the screw fasteners through the case for both terminals. These allow you to screw the ends of the cable into the back of a deck and the adapter for a secure connection.

Once completing these steps, you are ready to move on to testing your [cable and connection!](cableTesting.html).


