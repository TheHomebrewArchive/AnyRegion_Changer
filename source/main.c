/*-------------------------------------------------------------
 
regionchange.c -- Region Changing application
 
Copyright (C) 2008 tona
 
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.
 
Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:
 
1.The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.
 
2.Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.
 
3.This notice may not be removed or altered from any source
distribution.
 
-------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

#include "wiibasics.h"
#include "patchmii_core.h"
#include "sysconf.h"
#include "id.h"
#include "detect_settings.h"

#define ITEMS 	12
#define SADR_LENGTH 0x1007+1
#define WARNING_SIGN "\x1b[30;1m\x1b[43;1m/!\\\x1b[37;1m\x1b[40m"

u32 selected = 0;
char page_contents[ITEMS][64];

int lang, area, game, video, region, country, eula;
int using_temp_ios = 0;
u8 sadr[SADR_LENGTH];

char languages[][11] = {"Japanese  ", "English  ", "German  ", "French  ", "Spanish  ", "Italian  ", "Dutch   "};
char areas[][11] = {"Japan   ", "USA    ", "Europe  ", "Australia ", "Brazil  ", "Taiwan  ", "China   ", "Korea   ", "Hong Kong ", "Asia    ", "Latin Am. ", "S. Africa "};
char regions[][11] = {"Japan   ", "USA    ", "Europe  "};
char vmodes[][11] = {"NTSC   ", "PAL    ", "MPAL   "};
char eulas[][11] = {"Unread  ", "Read   "};

void handleError(const char* string, int errorval){
	printf("Unexpected Error: %s Value: %d\n", string, errorval);
	printf("Press any key to quit\n");
	wait_anyKey();
	exit(1);
}

void getSettings(void){
	int ret;
	lang = SYSCONF_GetLanguage();
	area = SYSCONF_GetArea();
	game = SYSCONF_GetRegion(); 
	video = SYSCONF_GetVideo();
	eula = SYSCONF_GetEULA();
	if (lang < 0 || area < 0 || game < 0 || video < 0 || (eula != SYSCONF_ENOENT && eula < 0)){
		printf("Error getting settings! %d, %d, %d, %d, %d\n", lang, area, game, video, eula);
		wait_anyKey();
		exit(1);
	}
	
	if (SYSCONF_GetLength("IPL.SADR") != SADR_LENGTH) handleError("IPL.SADR Length Incorrect", SYSCONF_GetLength("IPL.SADR"));
	ret = SYSCONF_Get("IPL.SADR", sadr, SADR_LENGTH);
	if (ret < 0 ) handleError("SYSCONF_Get IPL.SADR", ret);
	country = sadr[0];
	
}

void saveSettings(void){
	int ret = 0;
	if (lang != SYSCONF_GetLanguage()) ret = SYSCONF_SetLanguage(lang);
	if (ret) handleError("SYSCONF_SetLanguage", ret);
	if (area != SYSCONF_GetArea()) ret = SYSCONF_SetArea(area);
	if (ret) handleError("SYSCONF_SetArea", ret);
	if(game != SYSCONF_GetRegion()) ret = SYSCONF_SetRegion(game);
	if (ret) handleError("SYSCONF_SetRegion", ret);
	if (video != SYSCONF_GetVideo()) ret  = SYSCONF_SetVideo(video);
	if (ret) handleError("SYSCONF_SetVideo", ret);
	if (eula != SYSCONF_GetEULA()) ret  = SYSCONF_SetEULA(eula);
	if (ret) handleError("SYSCONF_SetEULA", ret);
		
	if (country != sadr[0]){
		memset(sadr, 0, SADR_LENGTH);
		sadr[0] = country;
		ret = SYSCONF_Set("IPL.SADR", sadr, SADR_LENGTH);
		if (ret) handleError("SYSCONF_Set IPL.SADR", ret);
	}
	//wait_anyKey();
	printf("Saving...");
	ret = SYSCONF_SaveChanges();
	if (ret < 0) handleError("SYSCONF_SaveChanges", ret);
	else printf("OK!\n");
	printf("Press any key to continue...\n");
	wait_anyKey();
}

void updateSelected(int delta){
	if (selected + delta >= ITEMS || selected + delta < 0) return;
	
	if (delta != 0){
		// Remove the cursor from the last selected item
		page_contents[selected][2] = ' ';
		page_contents[selected][45] = ' ';
		page_contents[selected][57] = ' ';
		// Set new cursor location
		selected += delta;
	}
	
	// Add the cursor to the now-selected item
	page_contents[selected][2] = '>';
	page_contents[selected][45] = '<';
	page_contents[selected][57] = '>';
}

void updatePage(void){
	
	sprintf(page_contents[0], "    %-40s   %10s  \n", "Language Setting:", languages[lang]);
	sprintf(page_contents[1], "    %-40s   %10s  \n", "Console Area Setting:", areas[area]);
	sprintf(page_contents[2], "    %-40s   %10s  \n", "Game Region Setting:", regions[game]);
	sprintf(page_contents[3], "    %-40s   %10s  \n", "Console Video Mode:", vmodes[video]);
	sprintf(page_contents[4], "    %-40s      %3d      \n", "Shop Country Code:", country);
	sprintf(page_contents[5], "    %-40s   %10s  \n", "Services EULA:", (eula == SYSCONF_ENOENT)?"Disabled ":eulas[eula]);
	sprintf(page_contents[6], "    %-40s   %10s  \n", "Revert Settings", "Revert  ");
	sprintf(page_contents[7], "    %-40s   %10s  \n", "Save Settings", "Save   ");
	sprintf(page_contents[8], "    %-40s   %10s  \n", "System Menu Region", regions[region]);
	sprintf(page_contents[9], "    %-40s   %10s  \n", "Install System Menu 3.2", "Go     ");
	sprintf(page_contents[10], "    %-40s   %10s  \n", "Exit to the Homebrew Channel", "Exit   ");
	sprintf(page_contents[11], "    %-40s   %10s  \n", "Reboot to System Menu", "Reboot  ");

	updateSelected(0);
}


int install_sysmenu(u32 version){
	int ret = 0, use_ios_patch = 0;
	
	if (get_installed_title_version(TITLE_ID(1,2)) >= version || 
		get_installed_title_version(TITLE_ID(1,30)) > 1040)
		use_ios_patch = 1;
	
	printf("\x1b[2J\n\n");
	patchmii_network_init();
	if (use_ios_patch && !using_temp_ios){
		printf("Installing temporary IOS35 for IOS/SysMenu downgrade...\n");
		ret = install_temporary_ios(35, 0);
		if (ret) goto cleanup;
		printf("Loading temporary IOS...\n");
		WPAD_Shutdown();
		load_temporary_ios();
		printf("\nLoaded IOS %d\n", IOS_GetVersion());
		using_temp_ios = 1;
		printf("Reinitializing...");
		miscInit();
		patchmii_network_init();
	}
	if(get_installed_title_version(TITLE_ID(1,30)) != 1040)
		ret = patchmii_install(1, 30, 1040, 1, 30, 1040, 0);
	if (ret) goto cleanup;
	ret = patchmii_install(1, 2, version, 1, 2, version, 0);
	if (ret) goto cleanup;
	
	cleanup:
	if(use_ios_patch) cleanup_temporary_ios();
	
	printf("Press any key to continue");
	wait_anyKey();
	return ret;
}

char AREAtoSysMenuRegion(int area){
	// Data based on my own tests with AREA/Sysmenu
	switch (area){
		case 0:
		case 5:
		case 6:
			return 'J';
		case 1:
		case 4:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
			return 'U';
		case 2:
		case 3:
			return 'E';
		default:
			return 0;
	}
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	int ret = 0, i;
	u16 sysmenu_version;
	char sysmenu_region;
	
	u32 buttons;
	
	//ret = IOS_ReloadIOS(35);
	
	basicInit();
	
	miscInit();
	//IdentSysMenu();
	
	//if(IOS_GetVersion() != 35) handleError("IOS35 not installed!", IOS_GetVersion());
	
	sysmenu_version = get_installed_title_version(TITLE_ID(1,2));
	sysmenu_region = get_sysmenu_region();
	printf("Init SYSCONF...");
	ret = SYSCONF_Init();
	if (ret < 0) handleError("SYSCONF_Init", ret);
	else printf("OK!\n");
	
	//wait_anyKey();
	//SYSCONF_PrintAllSettings();
	
	
	getSettings();
	region = game;

	printf("\x1b[2J\n");
	printf("\n\t\t\t\t\t  AnyRegion Changer 1.1\n");
	printf("\n\t  This software comes supplied with absolutely no warranty.\n");
	printf("\t\t\t\tUse this software at your own risk.\n");
	
	printf("\n\n\n\t\t\t\t" WARNING_SIGN " IMPORTANT BRICK INFORMATION " WARNING_SIGN "\n");
	printf("\n\tSemi Bricks are caused by the Console Area setting not matching\n");
	printf("\tyour System Menu region. A semi-brick in itself is not terribly\n");
	printf("\tharmful, but it can easily deteriorate into a full brick--there\n");
	printf("\tare multiple simple triggers to do so.\n");
	printf("\n\tIn order to practice proper safety when using this application, \n");
	printf("\tplease make sure that your Console Area and System Menu region \n");
	printf("\tare congruent before rebooting your system. A warning will be\n");
	printf("\tdisplayed if they are not in agreement.\n");
	
	sleep(5);
	printf("\n\n\t\t\t  Press (1) to continue or HOME to exit.\n");
	wait_key(WPAD_BUTTON_1);
	
	updatePage();
	
	main_loop:
	if (!sysmenu_region) sysmenu_region = '.';
	
	printf("\x1b[2J\n");
	//printf("---------------------------------------------------------------\n");
	printf("AnyRegion Changer 1.1b\n");
	printf("---------------------------------------------------------------\n");
	printf("Edit Region Settings");
	if (sysmenu_region != 0 && sysmenu_region != AREAtoSysMenuRegion(area))
		printf("    " WARNING_SIGN " \x1b[41;1mWARNING: AREA/SysMenu MISMATCH!\x1b[40m " WARNING_SIGN);
	printf("\n---------------------------------------------------------------\n");
	for (i = 0; i < 4; i++)
		printf(page_contents[i]);
	
	printf("    Country Codes:       1=JPN 49=USA 110=UK\n");
	
	for (i = i; i < 8; i++)
		printf(page_contents[i]);
	
	printf("---------------------------------------------------------------\n");
	printf("System Menu 3.2 Installer                 Installed Region: %c\n", sysmenu_region);
	printf("---------------------------------------------------------------\n");
	
	for (i = i; i < 10; i++)
		printf(page_contents[i]);
	
	printf("---------------------------------------------------------------\n");
	
	for (i = i; i < ITEMS; i++)
		printf(page_contents[i]);
	printf("---------------------------------------------------------------\n");
	
		
	
	buttons = wait_anyKey();
	
	if (buttons & WPAD_BUTTON_DOWN)
		updateSelected(1);
	
	if (buttons & WPAD_BUTTON_UP)
		updateSelected(-1);
	
	if (buttons & WPAD_BUTTON_LEFT){
		switch(selected){
			case 0:
				if (--lang < 0) lang = 6;
			break;
			case 1:
				if (--area < 0) area = 11;
			break;
			case 2:
				if (--game < 0) game = 2;
			break;
			case 3:
				if (--video < 0) video = 2;
			break;
			case 4:
				if (--country < 0) country = 255;
				if (eula >= 0) eula = 0;
			break;
			case 5:
				if (eula >= 0) eula = !eula;
			break;
			case 8:
				if (--region < 0) region = 2;
			break;
		}
	}
	
	if (buttons & WPAD_BUTTON_RIGHT){
		switch(selected){
			case 0:
				if (++lang == 7) lang = 0;
			break;
			case 1:
				if (++area == 12) area = 0;
			break;
			case 2:
				if (++game == 3) game = 0;
			break;
			case 3:
				if (++video == 3) video = 0;
			break;
			case 4:
				if (++country == 256) country = 0;
				if (eula >= 0) eula = 0;
			break;
			case 5:
				if (eula >= 0)	eula = !eula;
			break;
			case 8:
				if (++region == 3) region = 0;
			break;
		}
	}
	
	if (buttons & WPAD_BUTTON_A){
		switch(selected){
			case 4:
				//changeCountry();
			break;
			case 6:
				getSettings();
			break;
			case 7:
				saveSettings();
			break;
			case 9:
				printf("Are you sure you want to install System Menu 3.2%c?\n", regions[region][0]);
				if (yes_or_no()){
					install_sysmenu(288+region);
					sysmenu_region = get_sysmenu_region();
				}
			break;
			case 10:
				goto cleanup;
			break;
			case 11:
				STM_RebootSystem();
			break;
		}
	}
			
	
	
	updatePage();
	
	goto main_loop;

	//	SYSCONF_DumpBuffer();
	
	cleanup:
	miscDeInit();
	
	//STM_RebootSystem();
	return 0;
}
