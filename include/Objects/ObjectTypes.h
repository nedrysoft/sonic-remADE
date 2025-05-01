/**
 * Copyright (C) 2025 Adrian Carpenter
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * This file is part of Sonic remADE, a C++ implementation of a
 * Sonic The Hedgehog 1 game engine.
 *
 * https://github.com/nedrysoft/sonic-remade
 */

#ifndef NEDRYSOFT_OBJECTTYPES_H
#define NEDRYSOFT_OBJECTTYPES_H

/*
    objectNames[0x00] = "sonic";
    objectNames[0x0d] = "end-of-level-signpost";
    objectNames[0x11] = "bridge";
    objectNames[0x18] = "platform";
    objectNames[0x1a] = "collapsing-ledge";

    objectNames[0x1c] = "scenery";
    objectNames[0x1f] = "enemy-crabmeat";

    objectNames[0x22] = "enemy-buzz-bomber";
    objectNames[0x25] = "rings";
    objectNames[0x26] = "monitor";
    objectNames[0x2b] = "enemy-chopper";
    objectNames[0x36] = "spikes";
    objectNames[0x3b] = "purple-rock";
    objectNames[0x40] = "enemy-moto-bug";
    objectNames[0x41] = "spring";
    objectNames[0x42] = "enemy-newtron";
    objectNames[0x44] = "wall-barrier";
    objectNames[0x49] = "waterfall-sound";
    objectNames[0x4b] = "giant-ring";
    objectNames[0x79] = "lamp-post";
    objectNames[0x7d] = "hidden-points";
*/

enum class Objects {
	Sonic = 0x01,
	Unknown_1,
	Unknown_2,
	Unknown_3,
	Unknown_4,
	Unknown_5,
	Unknown_6,
	Water_Splash_LZ,
	Sonic_SS,
	Drowning_Counting_Numbers,
	Vertical_Pole_LZ,
	Flapping_Doors_LZ,
	End_Of_Level_Signpost,
	Sonic_Title_Screen,
	Press_Start_Button_And_TM,
	Blank_Object_1,
	Bridge_GHZ,
	Spinning_Light_SYZ,
	FireballMaker_MZ = 0x13,
	FireballMaker_SLZ = 0x13,
	Fireball_MZ = 0x14,
	Fireball_SLZ = 0x14,
	Swinging_Platforms_GHZ = 0x15,
	Swinging_Platforms_MZ = 0x15,
	Swinging_Platforms_SLZ = 0x15,
	Spiked_Ball_On_Chain_SBZ = 0x15,
	Spear_LZ,
	Helix_Of_Spikes_GHZ,
	Platforms_GHZ = 0x18,
	Platforms_SLZ = 0x18,
	Platforms_SYZ = 0x18,
	Blank_Object_2,
	Collapsing_Ledge_GHZ = 0x1A,
	Water_Surface_LZ,
	Scenery,
	Unused_1,
	Ballhog,
	Crabmeat,
	Cannonball,
	Score_Time_Rings,
	Buzz_Bomber,
	Buzz_Bomber_Missile,
	Buzz_Bomber_Dissolve,
	Ring,
	Monitor,
	Standard_Explosion,
	Animals,
	Points,
	One_Way_Barrier_SBZ,
	Enemy_Chopper,
	Enemy_Jaws,
	Enemy_Burrobot,
	Monitor_Damaged,
	Large_Platforms_MZ,
	Large_Glassy_Blocks_MZ,
	Stomping_Blocks_On_Chains,
	Switch,
	Moveable_Block_MZ = 0x33,
	Moveable_Block_LZ = 0x33,
	Title_Card,
	Fireball_Floor_MZ,
	Spikes,
	Rings_Hit,
	Shield_And_Invincibility_Stars,
	Game_Over = 0x39,
	Time_Over = 0x39,
	End_Of_Level_Results,
	Purple_Rock_GHZ,
	Breakable_Walls_GHZ = 0x3C,
	Breakable_Walls_SLZ = 0x3C,
	Eggman_GHZ,
	Prison,
	Large_Explosion,
	Enemy_Motobug,
	Springs,
	Enemy_Newtron_GHZ,
	Enemy_Roller_SYZ,
	Wall_Barrier_GHZ,
	Spiked_Metal_Block_MZ,
	Solid_Blocks_Falling_MZ,
	Bumper,
	Swinging_Ball_On_Chain_GHZ,
	Waterfall_Sound,
	Entry_SS,
	Giant_Ring,
	Lava_Geyser_MZ,
	Lava_Geyser_Ceiling,
	Wall_Of_Lava_MZ,
	Blank_Object_3,
	Enemy_Yadrin_SYZ,
	Smashable_Green_Block_MZ,
	Moving_Blocks_1_MZ = 0x52,
	Moving_Blocks_1_LZ = 0x52,
	Moving_Blocks_1_SBZ = 0x52,
	Collapsing_Floors_MZ = 0x53,
	Collapsing_Floors_SLZ = 0x53,
	Collapsing_Floors_LZ = 0x53,
	Invisible_Lava_Tag_MZ,
	Enemy_Basaran_MZ,
	Moving_Blocks_2_SYZ = 0x56,
	Moving_Blocks_2_SLZ = 0x56,
	Moving_Blocks_2_LZ = 0x56,
	Spiked_Ball_On_Chain,
	Spike_Ball,
	Raising_Platforms_SLZ,
	Platforms_Rotating_SLZ,
	Blocks_Staircase_SLZ,
	Girders_SLZ,
	Fans_SLZ,
	Seesaws_SLZ,
	Enemy_Bomb_SLZ = 0x5F,
	Enemy_Bomb_SBZ = 0x5F,
	Enemy_Orbinaut_LZ = 0x60,
	Enemy_Orbinaut_SLZ = 0x60,
	Enemy_Orbinaut_SBZ = 0x60,
	Blocks_LZ,
	Gargoyle_LZ,
	Platforms_Conveyor_LZ,
	Bubbles_LZ,
	Waterfalls_LZ,
	Rotating_Disc_SBZ,
	Disc_Run_Around_SBZ,
	Conveyor_Belts_SBZ,
	Spinning_Platforms_And_Conveyors_SBZ,
	Ground_And_Ceiling_Saws_SBZ,
	Stomper_And_Platforms_SBZ,
	Vanishing_Platforms_SBZ,
	Flamethrower_SBZ,
	Electrocution_SBZ,
	Spinning_Platforms_Around_Conveyor_SBZ,
	Large_Girder_Block_SBZ,
	Invisible_Solid_Blocks,
	Teleporter_SBZ,
	Eggman_SBZ,
	Lava_Eggman_Drops_MZ,
	Eggman_SYZ,
	Blocks_That_Eggman_Picks_Up_SYZ,
	Eggman_LZ,
	Enemy_Caterkiller_MZ = 0x78,
	Enemy_Caterkiller_SBZ = 0x78,
	Lamppost,
	Eggman_SLZ,
	Exploding_Spikeys,
	Giant_Ring_Flash,
	Hidden_Points,
	Results_Screen_SS,
	Results_Screen_Chaos_Emeralds,
	Continue_Screen,
	Sonic_On_Continue_Screen,
	Eggman_SBZ2,
	Disintegrating_Blocks_Eggman,
	Clyinders_Eggman_FZ,
	Eggman_FZ,
	EnergyBalls_Launcher_FZ,
	Ending_Sequence_Sonic,
	Ending_Sequence_Chaos_Emeralds,
	Ending_Sequence_Sonic_Hedgehog_Text,
	Sonic_Team_Presents_And_Credits,
	End_Screen_Try_Again_And_End,
	End_Screen_Try_Again_Chaos_Emeralds
};

#endif //NEDRYSOFT_OBJECTTYPES_H
