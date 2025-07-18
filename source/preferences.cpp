/****************************************************************************
 * FCE Ultra
 * Nintendo Wii/GameCube Port
 *
 * Tantric 2008-2022
 * Tanooki 2019-2023
 *
 * preferences.cpp
 *
 * Preferences save/load preferences utilities
 ****************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ogcsys.h>
#include <mxml.h>

#include "fceuxtx.h"
#include "filelist.h"
#include "button_mapping.h"
#include "filebrowser.h"
#include "menu.h"
#include "fileop.h"
#include "gcvideo.h"
#include "pad.h"

struct SGCSettings GCSettings;

/****************************************************************************
 * Prepare Preferences Data
 *
 * This sets up the save buffer for saving.
 ***************************************************************************/
static mxml_node_t *xml = NULL;
static mxml_node_t *data = NULL;
static mxml_node_t *section = NULL;
static mxml_node_t *item = NULL;
static mxml_node_t *elem = NULL;

static char temp[200];

static const char * toStr(int i)
{
	sprintf(temp, "%d", i);
	return temp;
}

static const char * FtoStr(float i)
{
	sprintf(temp, "%.2f", i);
	return temp;
}

static void createXMLSection(const char * name, const char * description)
{
	section = mxmlNewElement(data, "section");
	mxmlElementSetAttr(section, "name", name);
	mxmlElementSetAttr(section, "description", description);
}

static void createXMLSetting(const char * name, const char * description, const char * value)
{
	item = mxmlNewElement(section, "setting");
	mxmlElementSetAttr(item, "name", name);
	mxmlElementSetAttr(item, "value", value);
	mxmlElementSetAttr(item, "description", description);
}

static void createXMLController(u32 controller[], const char * name, const char * description)
{
	item = mxmlNewElement(section, "controller");
	mxmlElementSetAttr(item, "name", name);
	mxmlElementSetAttr(item, "description", description);

	// create buttons
	for(int i=0; i < MAXJP; i++)
	{
		elem = mxmlNewElement(item, "button");
		mxmlElementSetAttr(elem, "number", toStr(i));
		mxmlElementSetAttr(elem, "assignment", toStr(controller[i]));
	}
}

static const char * XMLSaveCallback(mxml_node_t *node, int where)
{
	const char *name;

	name = node->value.element.name;

	if(where == MXML_WS_BEFORE_CLOSE)
	{
		if(!strcmp(name, "file") || !strcmp(name, "section"))
			return ("\n");
		else if(!strcmp(name, "controller"))
			return ("\n\t");
	}
	if (where == MXML_WS_BEFORE_OPEN)
	{
		if(!strcmp(name, "file"))
			return ("\n");
		else if(!strcmp(name, "section"))
			return ("\n\n");
		else if(!strcmp(name, "setting") || !strcmp(name, "controller"))
			return ("\n\t");
		else if(!strcmp(name, "button"))
			return ("\n\t\t");
	}
	return (NULL);
}

static int
preparePrefsData ()
{
	xml = mxmlNewXML("1.0");
	mxmlSetWrapMargin(0); // disable line wrapping

	data = mxmlNewElement(xml, "file");
	mxmlElementSetAttr(data, "app", APPNAME);
	mxmlElementSetAttr(data, "version", APPVERSION);

	createXMLSection("File", "File Settings");

	createXMLSetting("AutoLoad", "Auto Load", toStr(GCSettings.AutoLoad));
	createXMLSetting("AutoSave", "Auto Save", toStr(GCSettings.AutoSave));
	createXMLSetting("LoadMethod", "Load Method", toStr(GCSettings.LoadMethod));
	createXMLSetting("SaveMethod", "Save Method", toStr(GCSettings.SaveMethod));
	createXMLSetting("LoadFolder", "Load Folder", GCSettings.LoadFolder);
	createXMLSetting("LastFileLoaded", "Last File Loaded", GCSettings.LastFileLoaded);
	createXMLSetting("SaveFolder", "Save Folder", GCSettings.SaveFolder);
	createXMLSetting("CheatFolder", "Cheats Folder", GCSettings.CheatFolder);
	createXMLSetting("gamegenie", "Game Genie", toStr(GCSettings.gamegenie));
	createXMLSetting("ScreenshotsFolder", "Screenshots Folder", GCSettings.ScreenshotsFolder);
	createXMLSetting("CoverFolder", "Covers Folder", GCSettings.CoverFolder);
	createXMLSetting("ArtworkFolder", "Artwork Folder", GCSettings.ArtworkFolder);

	createXMLSection("Video", "Video Settings");

	createXMLSetting("videomode", "Video Mode", toStr(GCSettings.videomode));
	createXMLSetting("zoomHor", "Horizontal Zoom Level", FtoStr(GCSettings.zoomHor));
	createXMLSetting("zoomVert", "Vertical Zoom Level", FtoStr(GCSettings.zoomVert));
	createXMLSetting("render", "Rendering", toStr(GCSettings.render));
	createXMLSetting("bilinear", "Bilinear Filtering", toStr(GCSettings.bilinear));
	createXMLSetting("aspect", "Aspect Ratio", toStr(GCSettings.aspect));
	createXMLSetting("hideoverscan", "Crop Overscan", toStr(GCSettings.hideoverscan));
	createXMLSetting("currpal", "Color Palette", toStr(GCSettings.currpal));
	createXMLSetting("ntsccolor", "NTSC Color", toStr(GCSettings.ntsccolor));
	createXMLSetting("crosshair", "Show Crosshair", toStr(GCSettings.crosshair));
	createXMLSetting("region", "Region", toStr(GCSettings.region));
	createXMLSetting("xshift", "Horizontal Video Shift", toStr(GCSettings.xshift));
	createXMLSetting("yshift", "Vertical Video Shift", toStr(GCSettings.yshift));

	createXMLSection("Audio", "Audio Settings");

	createXMLSetting("soundvolume", "Sound Volume", toStr(GCSettings.soundvolume));
	createXMLSetting("soundquality", "Sound Quality", toStr(GCSettings.soundquality));
	createXMLSetting("lowpass", "Low Pass Filter", toStr(GCSettings.lowpass));
	createXMLSetting("swapduty", "Swap Duty Cycles", toStr(GCSettings.swapduty));

	createXMLSection("Emulation Hacks", "Emulation Hacks Settings");

	createXMLSetting("overclock", "PPU Overclocking", toStr(GCSettings.overclock));
	createXMLSetting("nospritelimit", "No Sprite Limit", toStr(GCSettings.nospritelimit));

	createXMLSection("Menu", "Menu Settings");

#ifdef HW_RVL
	createXMLSetting("WiimoteOrientation", "Wiimote Orientation", toStr(GCSettings.WiimoteOrientation));
#endif
	createXMLSetting("ExitAction", "Exit Action", toStr(GCSettings.ExitAction));
	createXMLSetting("MusicVolume", "Music Volume", toStr(GCSettings.MusicVolume));
	createXMLSetting("SFXVolume", "Sound Effects Volume", toStr(GCSettings.SFXVolume));
	createXMLSetting("language", "Language", toStr(GCSettings.language));
	createXMLSetting("PreviewImage", "Preview Image", toStr(GCSettings.PreviewImage));
	createXMLSetting("HideRAMSaving", "Hide RAM Saving", toStr(GCSettings.HideRAMSaving));

	createXMLSection("Controller", "Controller Settings");

	createXMLSetting("Controller", "Controller", toStr(GCSettings.Controller));
	createXMLSetting("FastForward", "Fast Forward", toStr(GCSettings.FastForward));
	createXMLSetting("FastForwardButton", "Fast Forward Button", toStr(GCSettings.FastForwardButton));

	createXMLController(btnmap[CTRL_PAD][CTRLR_GCPAD], "btnmap_pad_gcpad", "NES Pad - GameCube Controller");
	createXMLController(btnmap[CTRL_PAD][CTRLR_WIIMOTE], "btnmap_pad_wiimote", "NES Pad - Wiimote");
	createXMLController(btnmap[CTRL_PAD][CTRLR_CLASSIC], "btnmap_pad_classic", "NES Pad - Classic Controller");
	createXMLController(btnmap[CTRL_PAD][CTRLR_WUPC], "btnmap_pad_wupc", "NES Pad - Wii U Pro Controller");
	createXMLController(btnmap[CTRL_PAD][CTRLR_WIIDRC], "btnmap_pad_wiidrc", "NES Pad - Wii U Gamepad");
	createXMLController(btnmap[CTRL_PAD][CTRLR_NUNCHUK], "btnmap_pad_nunchuk", "NES Pad - Nunchuk + Wiimote");
	createXMLController(btnmap[CTRL_ZAPPER][CTRLR_GCPAD], "btnmap_zapper_gcpad", "Zapper - GameCube Controller");
	createXMLController(btnmap[CTRL_ZAPPER][CTRLR_WIIMOTE], "btnmap_zapper_wiimote", "Zapper - Wiimote");

	int datasize = mxmlSaveString(xml, (char *)savebuffer, SAVEBUFFERSIZE, XMLSaveCallback);

	mxmlDelete(xml);

	return datasize;
}

/****************************************************************************
 * loadXMLSetting
 *
 * Load XML elements into variables for an individual variable
 ***************************************************************************/
static void loadXMLSetting(char * var, const char * name, int maxsize)
{
	item = mxmlFindElement(xml, xml, "setting", "name", name, MXML_DESCEND);
	if(item)
	{
		const char * tmp = mxmlElementGetAttr(item, "value");
		if(tmp)
			snprintf(var, maxsize, "%s", tmp);
	}
}
static void loadXMLSetting(int * var, const char * name)
{
	item = mxmlFindElement(xml, xml, "setting", "name", name, MXML_DESCEND);
	if(item)
	{
		const char * tmp = mxmlElementGetAttr(item, "value");
		if(tmp)
			*var = atoi(tmp);
	}
}
static void loadXMLSetting(float * var, const char * name)
{
	item = mxmlFindElement(xml, xml, "setting", "name", name, MXML_DESCEND);
	if(item)
	{
		const char * tmp = mxmlElementGetAttr(item, "value");
		if(tmp)
			*var = atof(tmp);
	}
}

/****************************************************************************
 * loadXMLController
 *
 * Load XML elements into variables for a controller mapping
 ***************************************************************************/
static void loadXMLController(u32 controller[], const char * name)
{
	item = mxmlFindElement(xml, xml, "controller", "name", name, MXML_DESCEND);

	if(item)
	{
		// populate buttons
		for(int i=0; i < MAXJP; i++)
		{
			elem = mxmlFindElement(item, xml, "button", "number", toStr(i), MXML_DESCEND);
			if(elem)
			{
				const char * tmp = mxmlElementGetAttr(elem, "assignment");
				if(tmp)
					controller[i] = atoi(tmp);
			}
		}
	}
}

/****************************************************************************
 * decodePrefsData
 *
 * Decodes preferences - parses XML and loads preferences into the variables
 ***************************************************************************/
static bool
decodePrefsData ()
{
	bool result = false;

	xml = mxmlLoadString(NULL, (char *)savebuffer, MXML_TEXT_CALLBACK);

	if(xml)
	{
		// check settings version
		item = mxmlFindElement(xml, xml, "file", "version", NULL, MXML_DESCEND);
		if(item) // a version entry exists
		{
			const char * version = mxmlElementGetAttr(item, "version");

			if(version && strlen(version) == 5)
			{
				// this code assumes version in format X.X.X
				// XX.X.X, X.XX.X, or X.X.XX will NOT work
				int verMajor = version[0] - '0';
				int verMinor = version[2] - '0';
				int verPoint = version[4] - '0';

				// check that the versioning is valid
				if(!(verMajor >= 1 && verMajor <= 9 &&
					verMinor >= 0 && verMinor <= 9 &&
					verPoint >= 0 && verPoint <= 9))
					result = false;
				else
					result = true;
			}
		}

		if(result)
		{
			// File Settings

			loadXMLSetting(&GCSettings.AutoLoad, "AutoLoad");
			loadXMLSetting(&GCSettings.AutoSave, "AutoSave");
			loadXMLSetting(&GCSettings.LoadMethod, "LoadMethod");
			loadXMLSetting(&GCSettings.SaveMethod, "SaveMethod");
			loadXMLSetting(GCSettings.LoadFolder, "LoadFolder", sizeof(GCSettings.LoadFolder));
			loadXMLSetting(GCSettings.LastFileLoaded, "LastFileLoaded", sizeof(GCSettings.LastFileLoaded));
			loadXMLSetting(GCSettings.SaveFolder, "SaveFolder", sizeof(GCSettings.SaveFolder));
			loadXMLSetting(GCSettings.CheatFolder, "CheatFolder", sizeof(GCSettings.CheatFolder));
			loadXMLSetting(&GCSettings.gamegenie, "gamegenie");
			loadXMLSetting(GCSettings.ScreenshotsFolder, "ScreenshotsFolder", sizeof(GCSettings.ScreenshotsFolder));
			loadXMLSetting(GCSettings.CoverFolder, "CoverFolder", sizeof(GCSettings.CoverFolder));
			loadXMLSetting(GCSettings.ArtworkFolder, "ArtworkFolder", sizeof(GCSettings.ArtworkFolder));

			// Video Settings

			loadXMLSetting(&GCSettings.videomode, "videomode");
			loadXMLSetting(&GCSettings.zoomHor, "zoomHor");
			loadXMLSetting(&GCSettings.zoomVert, "zoomVert");
			loadXMLSetting(&GCSettings.render, "render");
			loadXMLSetting(&GCSettings.bilinear, "bilinear");
			loadXMLSetting(&GCSettings.aspect, "aspect");
			loadXMLSetting(&GCSettings.hideoverscan, "hideoverscan");
			loadXMLSetting(&GCSettings.currpal, "currpal");
			loadXMLSetting(&GCSettings.ntsccolor, "ntsccolor");
			loadXMLSetting(&GCSettings.crosshair, "crosshair");
			loadXMLSetting(&GCSettings.region, "region");
			loadXMLSetting(&GCSettings.xshift, "xshift");
			loadXMLSetting(&GCSettings.yshift, "yshift");

			// Audio Settings

			loadXMLSetting(&GCSettings.soundvolume, "soundvolume");
			loadXMLSetting(&GCSettings.soundquality, "soundquality");
			loadXMLSetting(&GCSettings.lowpass, "lowpass");
			loadXMLSetting(&GCSettings.swapduty, "swapduty");

			// Emulation Hacks Settings

			loadXMLSetting(&GCSettings.overclock, "overclock");
			loadXMLSetting(&GCSettings.nospritelimit, "nospritelimit");

			// Menu Settings

			loadXMLSetting(&GCSettings.WiimoteOrientation, "WiimoteOrientation");
			loadXMLSetting(&GCSettings.ExitAction, "ExitAction");
			loadXMLSetting(&GCSettings.MusicVolume, "MusicVolume");
			loadXMLSetting(&GCSettings.SFXVolume, "SFXVolume");
			loadXMLSetting(&GCSettings.language, "language");
			loadXMLSetting(&GCSettings.PreviewImage, "PreviewImage");
			loadXMLSetting(&GCSettings.HideRAMSaving, "HideRAMSaving");			

			// Controller Settings

			loadXMLSetting(&GCSettings.Controller, "Controller");
			loadXMLSetting(&GCSettings.FastForward, "FastForward");
			loadXMLSetting(&GCSettings.FastForwardButton, "FastForwardButton");

			loadXMLController(btnmap[CTRL_PAD][CTRLR_GCPAD], "btnmap_pad_gcpad");
			loadXMLController(btnmap[CTRL_PAD][CTRLR_WIIMOTE], "btnmap_pad_wiimote");
			loadXMLController(btnmap[CTRL_PAD][CTRLR_CLASSIC], "btnmap_pad_classic");
			loadXMLController(btnmap[CTRL_PAD][CTRLR_WUPC], "btnmap_pad_wupc");
			loadXMLController(btnmap[CTRL_PAD][CTRLR_WIIDRC], "btnmap_pad_wiidrc");
			loadXMLController(btnmap[CTRL_PAD][CTRLR_NUNCHUK], "btnmap_pad_nunchuk");
			loadXMLController(btnmap[CTRL_ZAPPER][CTRLR_GCPAD], "btnmap_zapper_gcpad");
			loadXMLController(btnmap[CTRL_ZAPPER][CTRLR_WIIMOTE], "btnmap_zapper_wiimote");
		}
		mxmlDelete(xml);
	}
	return result;
}

/****************************************************************************
 * FixInvalidSettings
 *
 * Attempts to correct at least some invalid settings - the ones that
 * might cause crashes
 ***************************************************************************/
void FixInvalidSettings()
{
	if(GCSettings.LoadMethod > 6)
		GCSettings.LoadMethod = DEVICE_AUTO;
	if(GCSettings.SaveMethod > 6)
		GCSettings.SaveMethod = DEVICE_AUTO;
	if(!(GCSettings.zoomHor > 0.5 && GCSettings.zoomHor < 1.5))
		GCSettings.zoomHor = 1.0;
	if(!(GCSettings.zoomVert > 0.5 && GCSettings.zoomVert < 1.5))
		GCSettings.zoomVert = 1.0;
	if(!(GCSettings.xshift > -50 && GCSettings.xshift < 50))
		GCSettings.xshift = 0;
	if(!(GCSettings.yshift > -50 && GCSettings.yshift < 50))
		GCSettings.yshift = 0;
	if(!(GCSettings.MusicVolume >= 0 && GCSettings.MusicVolume <= 100))
		GCSettings.MusicVolume = 80;
	if(!(GCSettings.SFXVolume >= 0 && GCSettings.SFXVolume <= 100))
		GCSettings.SFXVolume = 20;
	if(GCSettings.language < 0 || GCSettings.language >= LANG_LENGTH)
		GCSettings.language = LANG_ENGLISH;
	if(GCSettings.Controller > CTRL_PAD4 || GCSettings.Controller < CTRL_ZAPPER)
		GCSettings.Controller = CTRL_PAD2;
	if(!(GCSettings.soundvolume >= 0 && GCSettings.soundvolume <= 150))
		GCSettings.soundvolume = 100;
	if(!(GCSettings.videomode >= 0 && GCSettings.videomode < 6))
		GCSettings.videomode = 0;
	if(!(GCSettings.render >= 0 && GCSettings.render < 2))
		GCSettings.render = 0;
	if(GCSettings.region < 0 || GCSettings.region > 2)
		GCSettings.region = 2;
}

/****************************************************************************
 * DefaultSettings
 *
 * Sets all the defaults!
 ***************************************************************************/
void
DefaultSettings ()
{
	memset (&GCSettings, 0, sizeof (GCSettings));

	ResetControls(); // Controller button mappings

	GCSettings.Controller = CTRL_PAD2; // NES Controllers, NES Zapper
	GCSettings.FastForward = 1; // Enabled by default
	GCSettings.FastForwardButton = 0; // Right analog stick

	GCSettings.gamegenie = 0; // Disabled by default

	GCSettings.soundvolume = 100; // Sound volume at 100%
	GCSettings.soundquality = 0; // Low sound quality
	GCSettings.lowpass = 0; // Disabled by default
	GCSettings.swapduty = 0; // Disabled by default

	GCSettings.overclock = 0; // Disabled by default
	GCSettings.nospritelimit = 0; // Disabled by default

	GCSettings.videomode = 0; // Automatic video mode detection
	GCSettings.render = 0; // Default rendering mode
	GCSettings.bilinear = 0; // Disabled by default
	GCSettings.hideoverscan = 1; // Hide vertical
	GCSettings.currpal = 0; // Default color palette
	GCSettings.ntsccolor = 0; // Disabled by default
	GCSettings.crosshair = 1; // Enabled by default
	GCSettings.region = 2; // Automatic region detection

	GCSettings.aspect = 0;

#ifdef HW_RVL
	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9)
		GCSettings.aspect = 1;
#endif

	GCSettings.zoomHor = 1.0; // Horizontal zoom level
	GCSettings.zoomVert = 1.0; // Vertical zoom level
	GCSettings.xshift = 0; // Horizontal video shift
	GCSettings.yshift = 0; // Vertical video shift

	GCSettings.WiimoteOrientation = 0;
	GCSettings.AutoloadGame = 0;
	GCSettings.ExitAction = 0; // Auto
	GCSettings.MusicVolume = 80;
	GCSettings.SFXVolume = 20;
	GCSettings.PreviewImage = 0;
	GCSettings.HideRAMSaving = 0;
	
#ifdef HW_RVL
	GCSettings.language = CONF_GetLanguage();
	
	if(GCSettings.language == LANG_TRAD_CHINESE)
		GCSettings.language = LANG_SIMP_CHINESE;
#else
	GCSettings.language = LANG_ENGLISH;
#endif

	GCSettings.LoadMethod = DEVICE_AUTO; // Auto, SD, DVD, USB
	GCSettings.SaveMethod = DEVICE_AUTO; // Auto, SD, USB
	sprintf (GCSettings.LoadFolder, "%s/roms", APPFOLDER); // Path to game files
	sprintf (GCSettings.SaveFolder, "%s/saves", APPFOLDER); // Path to save files
	sprintf (GCSettings.CheatFolder, "%s/cheats", APPFOLDER); // Path to cheat files
	sprintf (GCSettings.ScreenshotsFolder, "%s/screenshots", APPFOLDER); // Path to screenshots files
	sprintf (GCSettings.CoverFolder, "%s/covers", APPFOLDER); // Path to cover files
	sprintf (GCSettings.ArtworkFolder, "%s/artwork", APPFOLDER); // Path to artwork files
	GCSettings.AutoLoad = 1; // Auto Load RAM
	GCSettings.AutoSave = 1; // Auto Save RAM
}

/****************************************************************************
 * Save Preferences
 ***************************************************************************/
static char prefpath[MAXPATHLEN] = { 0 };

bool
SavePrefs (bool silent)
{
	char filepath[MAXPATHLEN];
	int datasize;
	int offset = 0;
	int device = 0;

	if(prefpath[0] != 0)
	{
		sprintf(filepath, "%s/%s", prefpath, PREF_FILE_NAME);
		FindDevice(filepath, &device);
	}
	else if(appPath[0] != 0)
	{
		sprintf(filepath, "%s/%s", appPath, PREF_FILE_NAME);
		strcpy(prefpath, appPath);
		FindDevice(filepath, &device);
	}
	else
	{
		device = autoSaveMethod(silent);

		if(device == 0)
			return false;
		
		sprintf(filepath, "%s%s", pathPrefix[device], APPFOLDER);
		
		DIR *dir = opendir(filepath);
		if (!dir)
		{
			if(mkdir(filepath, 0777) != 0)
				return false;
			sprintf(filepath, "%s%s/roms", pathPrefix[device], APPFOLDER);
			if(mkdir(filepath, 0777) != 0)
				return false;
			sprintf(filepath, "%s%s/saves", pathPrefix[device], APPFOLDER);
			if(mkdir(filepath, 0777) != 0)
				return false;
		}
		else
		{
			closedir(dir);
		}
		sprintf(filepath, "%s%s/%s", pathPrefix[device], APPFOLDER, PREF_FILE_NAME);
		sprintf(prefpath, "%s%s", pathPrefix[device], APPFOLDER);
	}

	if(device == 0)
		return false;

	if (!silent)
		ShowAction ("Saving preferences...");

	FixInvalidSettings();

	AllocSaveBuffer ();
	datasize = preparePrefsData ();
	offset = SaveFile(filepath, datasize, silent);

	FreeSaveBuffer ();

	CancelAction();

	if (offset > 0)
	{
		if (!silent)
			InfoPrompt("Preferences saved");
		return true;
	}
	return false;
}

/****************************************************************************
 * Load Preferences from specified filepath
 ***************************************************************************/
bool
LoadPrefsFromMethod (char * path)
{
	bool retval = false;
	int offset = 0;
	char filepath[MAXPATHLEN];
	sprintf(filepath, "%s/%s", path, PREF_FILE_NAME);

	AllocSaveBuffer ();

	offset = LoadFile(filepath, SILENT);

	if (offset > 0)
		retval = decodePrefsData ();

	FreeSaveBuffer ();

	if(retval)
	{
		strcpy(prefpath, path);

		if(appPath[0] == 0)
			strcpy(appPath, prefpath);
	}

	return retval;
}

/****************************************************************************
 * Load Preferences
 * Checks sources consecutively until we find a preference file
 ***************************************************************************/
static bool prefLoaded = false;

bool LoadPrefs()
{
	if(prefLoaded) // already attempted loading
		return true;

	bool prefFound = false;
	char filepath[5][MAXPATHLEN];
	int numDevices;
	bool sdMounted = false;
	bool usbMounted = false;

#ifdef HW_RVL
	numDevices = 4;
	sprintf(filepath[0], "%s", appPath);
	sprintf(filepath[1], "sd:/apps/%s", APPFOLDER);
	sprintf(filepath[2], "usb:/apps/%s", APPFOLDER);
	sprintf(filepath[3], "sd:/%s", APPFOLDER);
	sprintf(filepath[4], "usb:/%s", APPFOLDER);

	for(int i=0; i<numDevices; i++)
	{
		prefFound = LoadPrefsFromMethod(filepath[i]);
		
		if(prefFound)
			break;
	}
#else
	if(ChangeInterface(DEVICE_SD_SLOTA, SILENT)) {
		sprintf(filepath[0], "carda:/%s", APPFOLDER);
		prefFound = LoadPrefsFromMethod(filepath[0]);
	}
	else if(ChangeInterface(DEVICE_SD_SLOTB, SILENT)) {
		sprintf(filepath[0], "cardb:/%s", APPFOLDER);
		prefFound = LoadPrefsFromMethod(filepath[0]);
	}
	else if(ChangeInterface(DEVICE_SD_PORT2, SILENT)) {
		sprintf(filepath[0], "port2:/%s", APPFOLDER);
		prefFound = LoadPrefsFromMethod(filepath[0]);
	}
#endif

	prefLoaded = true; // attempted to load preferences

	if(prefFound)
		FixInvalidSettings();

	// rename fceuxtx to fceultrafx
	if(GCSettings.LoadMethod == DEVICE_SD)
	{
		sdMounted = ChangeInterface(DEVICE_SD, NOTSILENT);
		if(sdMounted && opendir("sd:/fceux"))
			rename("sd:/fceux", "sd:/fceultrafx");
	}
	else if(GCSettings.LoadMethod == DEVICE_USB)
	{
		usbMounted = ChangeInterface(DEVICE_USB, NOTSILENT);
		if(usbMounted && opendir("usb:/fceux"))
			rename("usb:/fceux", "usb:/fceultrafx");	
	}

	// update folder locations
	if(strcmp(GCSettings.LoadFolder, "fceux/roms") == 0)
		sprintf(GCSettings.LoadFolder, "fceultrafx/roms");
	
	if(strcmp(GCSettings.SaveFolder, "fceux/saves") == 0)
		sprintf(GCSettings.SaveFolder, "fceultrafx/saves");
	
	if(strcmp(GCSettings.CheatFolder, "fceux/cheats") == 0)
		sprintf(GCSettings.CheatFolder, "fceultrafx/cheats");
		
	if(strcmp(GCSettings.ScreenshotsFolder, "fceux/screenshots") == 0)
		sprintf(GCSettings.ScreenshotsFolder, "fceultrafx/screenshots");

	if(strcmp(GCSettings.CoverFolder, "fceux/covers") == 0)
		sprintf(GCSettings.CoverFolder, "fceultrafx/covers");
	
	if(strcmp(GCSettings.ArtworkFolder, "fceux/artwork") == 0)
		sprintf(GCSettings.ArtworkFolder, "fceultrafx/artwork");
	
	// attempt to create directories if they don't exist
	if((GCSettings.LoadMethod == DEVICE_SD && sdMounted) || (GCSettings.LoadMethod == DEVICE_USB && usbMounted) ) {
		char dirPath[MAXPATHLEN];
		sprintf(dirPath, "%s%s", pathPrefix[GCSettings.LoadMethod], GCSettings.ScreenshotsFolder);
		CreateDirectory(dirPath);
		sprintf(dirPath, "%s%s", pathPrefix[GCSettings.LoadMethod], GCSettings.CoverFolder);
		CreateDirectory(dirPath);
		sprintf(dirPath, "%s%s", pathPrefix[GCSettings.LoadMethod], GCSettings.ArtworkFolder);
		CreateDirectory(dirPath);
		sprintf(dirPath, "%s%s", pathPrefix[GCSettings.LoadMethod], GCSettings.CheatFolder);
		CreateDirectory(dirPath);
	}

	if(GCSettings.videomode > 0) {
		ResetVideo_Menu();
	}

#ifdef HW_RVL
	bg_music = (u8 * )bg_music_ogg;
	bg_music_size = bg_music_ogg_size;
	LoadBgMusic();
#endif

	ChangeLanguage();
	return prefFound;
}
