#this is the project page for my 12th grade finals project in mechatronics #
the main idea of the project is a robot that has a Mauwb radar antenna with two tags connected to two separate esp32's 
each of the tag esp's will have a screen, a joystick and a Mauwb tag
the bot will have the antenna and a lidar to spot the users and avoid obstacles
#what can the bot do in terms of commands#
the bot will have premade words to choose from for example
1: select user, usually the second word can come after something like go(second example) made to tell the bot who to act apon
2:go, will make the bot go to the selected target, selected target can be from selected user(example one) or me(third option)
3:me, another select user option, selects the current user for commands like (go second example) ect.
4:give,comes to the current user, opens the countainer, the user types what ther are giveing and then goes to the selected user(cant select me here) and the user can open the container with the tag esp(they will have a prompt telling them"user x has sent you ----(---- is the output if you dont type the name of what you are giveing)")
5:get, select a user then write what you need then the bot goes to that user and will open the container and give the selected user a prompt to put it with a button to close and put the item and the option to reject the request for the item
6:roam/wander,this is a single work command that does not accept any other tags like select user or me,this command makes the bot roam using googles fly brain scan program "https://blog.google/innovation-and-ai/technology/research/male-fruit-fly-brain-map/" im considering adding a camera so it could react with more then just a lidar and maybe a light sensor.
