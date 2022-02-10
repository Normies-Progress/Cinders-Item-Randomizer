Credit to LukeYui for the original Randomiser. You can find it here: https://github.com/LukeYui/DS3-Item-Randomiser-OS

Unzip the contents to your game directory, modifying the values in the randomizer preferences as desired. While setting a number higher than 2147 via the item order randomizer executable likely won't break it, it would be best to set it to the item count above.

Please note there may be bugs, as this is still a work in progress. Additionally, there may be some items that still need to be removed. If so, please make a note of the new item ID displayed in the debugger console.

If key item randomization is enabled, please be aware the odds of picking up a key item are lower than in vanilla (44+:1), and may result in being locked out of certain areas (i.e. Grand Archives) for a longer-than-usual period of time.

Use the Item Order Randomiser.exe to generate new seeds, use New and then Custom List, entering 1900 for the item number. The item list is built into the .dll, meaning the Data_Item_List.txt doesn't need to be configured.

Updated to 2.15