#include "Core.h"

CCore* Core;
CItemRandomiser *ItemRandomiser;
CAutoEquip *AutoEquip;
SCore* CoreStruct;

DWORD64 qItemEquipComms = 0;

DWORD64 rItemRandomiser = 0;
DWORD64 rAutoEquip = 0;
DWORD64 rNoWeaponRequirements = 0;
DWORD64 rEquipLock = 0;
//HARDCODED LIST
DWORD pItemArrList[2139];

VOID CCore::Start() {

	Core = new CCore();
	CoreStruct = new SCore();
	ItemRandomiser = new CItemRandomiser();
	AutoEquip = new CAutoEquip();

	Core->DebugInit();

	CoreStruct->hHeap = HeapCreate(8, 0x10000, 0);
	if (!CoreStruct->hHeap) {
		Core->Panic("Unable to allocate appropriate heap", "...\\Randomiser\\Core\\Core.cpp", FE_MemError, 1);
		int3
	};

	if (!Core->Initialise()){
		Core->Panic("Failed to initialise", "...\\Randomiser\\Core\\Core.cpp", FE_InitFailed, 1);
		int3
	};

	while (true) {
		Core->Run();
		Sleep(2500);
	};

	if (!HeapFree(CoreStruct->hHeap, 8, CoreStruct->pItemArray)) {
		Core->Panic("Given memory block appears invalid, or freed already", "...\\Randomiser\\Core\\Core.cpp", FE_InitFailed, 1);
		int3
	};

	HeapDestroy(CoreStruct->hHeap);

	delete AutoEquip;
	delete ItemRandomiser;
	delete CoreStruct;
	delete Core;

	return;
};

VOID CCore::Run() {

	if ((CoreStruct->dIsAutoSave) && CoreStruct->dIsListChanged) {
		Core->SaveArrayList();
		CoreStruct->dIsListChanged--;
	};

	if (CoreStruct->dIsMessageActive) {
		DisplayInfoMsg();
	};

	return;
};

BOOL CCore::Initialise() {

	int i = 0;
	char pBuffer[MAX_PATH];
	BOOL bReturn = true;
	INIReader reader("RandomiserPreferences.ini");

	if (reader.ParseError() == -1) {
		MessageBoxA(NULL, "Failed to find 'RandomiserPreferences.ini'.", "Load Error", MB_ICONWARNING);
		int3
	};

	if (MH_Initialize() != MH_OK) return false;

	CoreStruct->dIsAutoSave = reader.GetBoolean("Randomiser", "SaveProgress", true);
	//CoreStruct->dRandomsieHealItems = reader.GetBoolean("Randomiser", "RandomiseHeals", true);
	CoreStruct->dRandomsieHealItems = reader.GetBoolean("Randomiser", "RandomEstusMaterials", true);
	CoreStruct->dRandomiseKeyItems = reader.GetBoolean("Randomiser", "RandomKeyItems", false);
	CoreStruct->dIsMessageActive = reader.GetBoolean("Randomiser", "RandomiserMessage", true);
	CoreStruct->dRandomiseCovenantItems = reader.GetBoolean("Randomiser", "RandomCovenantDrops", false);
	CoreStruct->dIsRandomInfusion = reader.GetBoolean("Randomiser", "AllowRandomInfusion", false);
	CoreStruct->dIsRandomReinforcement = reader.GetBoolean("Randomiser", "AllowRandomReinforcement", false);
	CoreStruct->dIsRandomAffixation = reader.GetBoolean("Randomiser", "AllowRandomAffixation", false);
	CoreStruct->dMinGoodsValue = reader.GetInteger("Values", "GoodsRandomMin", 1);
	CoreStruct->dMaxGoodsValue = reader.GetInteger("Values", "GoodsRandomMax", 1);
	CoreStruct->dIsAutoEquip = reader.GetBoolean("AutoEquip", "AutoEquipToggle", true);
	CoreStruct->dLockEquipSlots = reader.GetBoolean("AutoEquip", "LockEquipSlots", false);
	CoreStruct->dIsNoWeaponRequirements = reader.GetBoolean("AutoEquip", "NoWeaponRequirements", false);

	CoreStruct->pOffsetArray = (DWORD*)HeapAlloc(CoreStruct->hHeap, 8, 0x3000);
	CoreStruct->pItemArray = (DWORD*)HeapAlloc(CoreStruct->hHeap, 8, 0x3000);
	//0x3000
	//HARDCODED LIST
	CoreStruct->pItemArr = pItemArrList;

	//if ((!CoreStruct->pItemArray) || (!CoreStruct->pOffsetArray)) {
	//	Core->Panic("Out of memory", "...\\Randomiser\\Core\\Core.cpp", FE_MemError, 1);
	//	int3
	//};

	//HARDCODED LIST
	if ((!CoreStruct->pItemArr) || (!CoreStruct->pOffsetArray)) {
		Core->Panic("Out of memory", "...\\Randomiser\\Core\\Core.cpp", FE_MemError, 1);
		int3
	};

#ifdef DEBUG
	sprintf_s(pBuffer, "[Randomiser] - SaveProgress = %i\n", CoreStruct->dIsAutoSave);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Randomiser] - RandomiseHeals = %i\n", CoreStruct->dRandomsieHealItems);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Randomiser] - RandomiseKeys = %i\n", CoreStruct->dRandomiseKeyItems);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Randomiser] - RandomiseCovenantDrops = %i\n", CoreStruct->dRandomiseCovenantItems);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Randomiser] - RandomsierMessage = %i\n", CoreStruct->dIsMessageActive);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Randomiser] - AllowRandomisedInfisions = %i\n", CoreStruct->dIsRandomInfusion);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Randomiser] - AllowRandomisedReinforcement = %i\n", CoreStruct->dIsRandomReinforcement);
	printf_s(pBuffer);	
	sprintf_s(pBuffer, "[Randomiser] - AllowRandomAffixation = %i\n", CoreStruct->dIsRandomAffixation);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Values] - GoodsRandomMin = %i\n", CoreStruct->dMinGoodsValue);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Values] - GoodsRandomMax = %i\n", CoreStruct->dMaxGoodsValue);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[AutoEquip] - AutoEquipToggle = %i\n", CoreStruct->dIsAutoEquip);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[AutoEquip] - LockEquipSlots = %i\n", CoreStruct->dLockEquipSlots);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[AutoEquip] - NoWeaponRequirements = %i\n", CoreStruct->dIsNoWeaponRequirements);
	printf_s(pBuffer);
#endif

	GetArrayList();

	while (!CoreStruct->pOffsetArray[i+1]) {
		if (CoreStruct->pOffsetArray[0] == i) break;
		/*CoreStruct->pItemArray[0]++;*/
		//HARDCODED List
		CoreStruct->pItemArr[0]++;
		i++;
	}; 

	if (CoreStruct->dLockEquipSlots) {
		LockEquipSlots();
	};

	bReturn &= Hook(0x1407BBA80, (DWORD64)&tItemRandomiser, &rItemRandomiser, 5);

	if (CoreStruct->dIsAutoEquip) bReturn &= Hook(0x1407BBE92, (DWORD64)&tAutoEquip, &rAutoEquip, 6);
	if (CoreStruct->dIsNoWeaponRequirements) bReturn &= Hook(0x140C073B9, (DWORD64)&tNoWeaponRequirements, &rNoWeaponRequirements, 7);

	AutoEquip->EquipItem = (fEquipItem*)0x140AFBBB0;
	Core->DisplayGraveMessage = (fDisplayGraveMessage*)0x140BE1990;

	return bReturn;
};

BOOL CCore::GetArrayList() {

	DWORD i = 0;

	std::ifstream readfileA("Data_Item_Order.txt");
	std::ifstream readfileB("Data_Item_List.txt");

	DWORD* pOffsetList = CoreStruct->pOffsetArray;
	/*DWORD* pItemList = CoreStruct->pItemArray;*/
	//HARDCODED LIST
	DWORD* pItemList = CoreStruct->pItemArr;

	if (readfileA.is_open()) {

		readfileA >> pOffsetList[0];
		i++;

		while (i <= *pOffsetList) {
			readfileA >> pOffsetList[i];
			i++;
		};
		readfileA.close();

	}
	else MessageBoxA(NULL, "Failed to find 'Data_Item_Order.txt'", "Load Error", MB_ICONWARNING);

	i = 1;

	if (readfileB.is_open()) {

		while (i <= *pOffsetList) {
			readfileB >> std::hex >> pItemList[i];
			i++;
		};
		readfileB.close();
		return true;
	};

	MessageBoxA(NULL, "Failed to find 'Data_Item_List.txt'", "Load Error", MB_ICONWARNING);

	return false;
};

BOOL CCore::SaveArrayList() {

	DWORD i = 0;

	std::ofstream outfile("Data_Item_Order.txt");

	DWORD* pOffsetList = CoreStruct->pOffsetArray;
	/*DWORD* pItemList = CoreStruct->pItemArray;*/
	//HARDCODED LIST
	DWORD* pItemList = CoreStruct->pItemArr;

	if (outfile.is_open()) {

		while (i <= *pOffsetList) {
			outfile << pOffsetList[i] << std::endl;
			i++;
		};
		outfile.close();
		return true;
	};

	CoreStruct->dIsAutoSave = 0;
	MessageBoxA(NULL, "Failed to find 'Data_Item_Order.txt'", "Save Error", MB_ICONWARNING);


	return false;

};

BOOL CCore::Hook(DWORD64 qAddress, DWORD64 qDetour, DWORD64* pReturn, DWORD dByteLen) {

	if (MH_CreateHook((LPVOID)qAddress, (LPVOID)qDetour, 0) != MH_OK) return false;
	if (MH_EnableHook((LPVOID)qAddress) != MH_OK) return false;

	*pReturn = (qAddress + dByteLen);

	return true;
};

VOID CCore::Panic(char* pMessage, char* pSort, DWORD dError, DWORD dIsFatalError) {

	char pOutput[MAX_PATH];
	char pTitle[MAX_PATH];

	sprintf_s(pOutput, "%s -> %s (%i)", pSort, pMessage, dError);

	if (IsDebuggerPresent()) {
		OutputDebugStringA(pOutput);
	};

	if (CoreStruct->dIsDebug) {
		printf_s("CCore::Panic is outputting debug-mode error information\n");
		sprintf_s(pOutput, "%s\n", pOutput);
		printf_s(pOutput);
	}
	else {
		if (dIsFatalError){
			sprintf_s(pTitle, "[Item Randomiser - Fatal Error]");
		} 
		else {
			sprintf_s(pTitle, "[Item Randomiser - Error]");
		}; 
		
		MessageBoxA(NULL, pOutput, pTitle, MB_ICONERROR);
	};

	if (dIsFatalError) *(int*)0 = 0;

	return;
};

VOID CCore::DisplayInfoMsg() {
	/*
	UINT_PTR qLuaEvent = 0;
	UINT_PTR qWorldChrMan = 0;

	qLuaEvent = *(UINT_PTR*)CoreStruct->qSprjLuaEvent;
	if (!qLuaEvent) return;

	qWorldChrMan = *(UINT_PTR*)CoreStruct->qWorldChrMan;
	if (!qWorldChrMan) return;
	qWorldChrMan = *(UINT_PTR*)(qWorldChrMan + 0x80);
	if (!qWorldChrMan) return;

	if (!Core->DisplayGraveMessage) {
		Core->Panic("Bad function call", "...\\Source\\Core\\Core.cpp", FE_BadFunc, 1);
		int3
	};

	Core->DisplayGraveMessage(0x33333333);
	*/
	CoreStruct->dIsMessageActive = 0;

	return;
};

VOID CCore::LockEquipSlots() {

	DWORD dOldProtect = 0;
	DWORD64 qEquip = 0x140B70F45;
	DWORD64 qUnequip = 0x140B736EA;

	if (!VirtualProtect((LPVOID)qEquip, 1, PAGE_EXECUTE_READWRITE, &dOldProtect)) return;
	if (!VirtualProtect((LPVOID)qUnequip, 1, PAGE_EXECUTE_READWRITE, &dOldProtect)) return;

	*(BYTE*)qEquip = 0x30;
	*(BYTE*)qUnequip = 0x30;

	if (!VirtualProtect((LPVOID)qEquip, 1, dOldProtect, &dOldProtect)) return;
	if (!VirtualProtect((LPVOID)qUnequip, 1, dOldProtect, &dOldProtect)) return;

	return;
};

VOID CCore::DebugInit() {
	CoreStruct->dIsDebug = 0;
#ifdef DEBUG
	FILE* fp;

	AllocConsole();
	SetConsoleTitleA("Dark Souls III - Item Randomiser Debug Console");
	freopen_s(&fp, "CONOUT$", "w", stdout);
	printf_s("Starting...\n");
	CoreStruct->dIsDebug = 1;
#endif

	return;
};

extern DWORD pItemArrList[2139] = {

	//AMMO
	0x00061A80,
	0x00061AE4,
	0x00061B48,
	0x00061BAC,
	0x00061C10,
	0x00061CD8,
	0x00061D3C,
	0x00062250,
	0x00062318,
	0x0006237C,
	0x000623E0,
	0x00062444,
	0x00062A20,
	0x00062A84,
	0x00062AE8,
	0x00062B4C,
	0x00062BB0,
	0x00062C14,
	0x00062C78,
	0x00064190,
	0x000642BC,
	0x00064320,
	0x000643E8,
	0x00064960,
	0x00064A8C,
	0x00065130,
	0x00065194,
	0x00067070,
	0x00067840,
	0x00069780,
	0x00069F50,
	0x0006BE90,
	0x0006C660,
	0x0006ED70,
	//ARMOR
	0x1098BD90,
	0x1098C178,
	0x1098C560,
	0x1098C948,
	0x1098E4A0,
	0x1098E888,
	0x1098EC70,
	0x1098F058,
	0x10990BB0,
	0x10990F98,
	0x10991380,
	0x10991768,
	0x109932C0,
	0x109936A8,
	0x10993A90,
	0x10993E78,
	0x109959D0,
	0x10995DB8,
	0x109961A0,
	0x10996588,
	0x109980E0,
	0x109984C8,
	0x109988B0,
	0x10998C98,
	0x1099A7F0,
	0x1099ABD8,
	0x1099AFC0,
	0x1099B3A8,
	0x1099CF00,
	0x1099D2E8,
	0x1099D6D0,
	0x1099DAB8,
	0x1099F610,
	0x109A1D20,
	0x109A2108,
	0x109A24F0,
	0x109A28D8,
	0x109A4430,
	0x109A4818,
	0x109A4C00,
	0x109A4FE8,
	0x109A6B40,
	0x109A6F28,
	0x109A7310,
	0x109A76F8,
	0x109A9250,
	0x109A9638,
	0x109A9A20,
	0x109A9E08,
	0x109AB960,
	0x109ABD48,
	0x109AC130,
	0x109AC518,
	0x109AE070,
	0x109AE458,
	0x109AE840,
	0x109AEC28,
	0x109B0780,
	0x109B0B68,
	0x109B2E90,
	0x109B55A0,
	0x109B7CB0,
	0x109BA3C0,
	0x109BCAD0,
	0x109BF1E0,
	0x109C18F0,
	0x109C4000,
	0x109C6710,
	0x109C8E20,
	0x109CB530,
	0x109CDC40,
	0x109D0350,
	0x109D0738,
	0x109D0B20,
	0x109D0F08,
	0x109D2A60,
	0x109D2E48,
	0x109D3230,
	0x109D3618,
	0x109D5170,
	0x109D5558,
	0x109D5940,
	0x109D5D28,
	0x109D7880,
	0x109D7C68,
	0x109D8050,
	0x109D8438,
	0x109D9F90,
	0x109DA378,
	0x109DA760,
	0x109DAB48,
	0x109DC6A0,
	0x109DCA88,
	0x109DCE70,
	0x109DD258,
	0x109DEDB0,
	0x109DF198,
	0x109DF580,
	0x109DF968,
	0x109E14C0,
	0x109E18A8,
	0x109E1C90,
	0x109E2078,
	0x109E3BD0,
	0x109E3FB8,
	0x109E43A0,
	0x109E4788,
	0x109E66C8,
	0x109E6AB0,
	0x109E6E98,
	0x109E89F0,
	0x109E8DD8,
	0x109E91C0,
	0x109E95A8,
	0x109EB4E8,
	0x109EB8D0,
	0x109EBCB8,
	0x1121EAC0,
	0x1121EEA8,
	0x1121F290,
	0x1121F678,
	0x11298BE0,
	0x11298FC8,
	0x112993B0,
	0x11299798,
	0x11312D00,
	0x113130E8,
	0x113134D0,
	0x113138B8,
	0x11406F40,
	0x11407328,
	0x11407710,
	0x11407AF8,
	0x11481060,
	0x11481448,
	0x11481830,
	0x11481C18,
	0x114FB180,
	0x114FB568,
	0x114FB950,
	0x114FBD38,
	0x115752A0,
	0x11575688,
	0x11575A70,
	0x11575E58,
	0x115EF3C0,
	0x115EF7A8,
	0x115EFB90,
	0x115EFF78,
	0x11607A60,
	0x11607E48,
	0x116694E0,
	0x116698C8,
	0x11669CB0,
	0x1166A098,
	0x116E3600,
	0x116E39E8,
	0x116E3DD0,
	0x116E41B8,
	0x1175D720,
	0x1175DB08,
	0x1175DEF0,
	0x1175E2D8,
	0x117D7840,
	0x117D7C28,
	0x117D8010,
	0x117D83F8,
	0x11851960,
	0x11851D48,
	0x11852130,
	0x11852518,
	0x118CBA80,
	0x118CBE68,
	0x118CC250,
	0x118CC638,
	0x11945BA0,
	0x11945F88,
	0x11946370,
	0x11946758,
	0x119BFCC0,
	0x119C00A8,
	0x119C0490,
	0x119C0878,
	0x11A39DE0,
	0x11A3A1C8,
	0x11A3A5B0,
	0x11A3A998,
	0x11AB3F00,
	0x11AB42E8,
	0x11AB46D0,
	0x11AB4AB8,
	0x11B2E408,
	0x11B2E7F0,
	0x11B2EBD8,
	0x11BA8140,
	0x11BA8528,
	0x11BA8910,
	0x11BA8CF8,
	0x11C22260,
	0x11C22648,
	0x11C22A30,
	0x11C22E18,
	0x11C9C380,
	0x11C9C768,
	0x11C9CB50,
	0x11C9CF38,
	0x11CB4E08,
	0x11D905C0,
	0x11D909A8,
	0x11D90D90,
	0x11D91178,
	0x11E84800,
	0x11E84BE8,
	0x11E84FD0,
	0x11E853B8,
	0x11F78A40,
	0x11F78E28,
	0x11F79210,
	0x11F795F8,
	0x12625A00,
	0x12625DE8,
	0x126265B8,
	0x12656740,
	0x129020C0,
	0x129F6300,
	0x129F66E8,
	0x129F6AD0,
	0x129F6EB8,
	0x12A70420,
	0x12A70808,
	0x12AEA540,
	0x12AEA928,
	0x12AEAD10,
	0x12AEB0F8,
	0x12BDE780,
	0x12BDEB68,
	0x12BDEF50,
	0x12BDF338,
	0x12CD29C0,
	0x12CD2DA8,
	0x12CD3190,
	0x12CD3578,
	0x12D4CAE0,
	0x12DC6C00,
	0x12DC6FE8,
	0x12DC73D0,
	0x12DC77B8,
	0x12E40D20,
	0x12E41108,
	0x12EBAE40,
	0x12EBB228,
	0x12EBB610,
	0x12EBB9F8,
	0x130291A0,
	0x13029588,
	0x13029970,
	0x13029D58,
	0x13197500,
	0x131978E8,
	0x13197CD0,
	0x131980B8,
	0x1328B740,
	0x1328BB28,
	0x1328BF10,
	0x1328C2F8,
	0x1337F980,
	0x1337FD68,
	0x13380150,
	0x13380538,
	0x133F9AA0,
	0x133F9E88,
	0x133FA270,
	0x133FA658,
	0x13473BC0,
	0x13473FA8,
	0x13474390,
	0x13474778,
	0x134EDCE0,
	0x134EE0C8,
	0x134EE4B0,
	0x134EE898,
	0x13567E00,
	0x135681E8,
	0x135685D0,
	0x135689B8,
	0x135E1F20,
	0x135E2308,
	0x135E2AD8,
	0x1365C040,
	0x1365C428,
	0x1365C810,
	0x1365CBF8,
	0x136D6160,
	0x13750280,
	0x13750668,
	0x13750A50,
	0x13750E38,
	0x137CA3A0,
	0x137CA788,
	0x137CAB70,
	0x137CAF58,
	0x138BE5E0,
	0x138BE9C8,
	0x13938700,
	0x13938AE8,
	0x13938ED0,
	0x139392B8,
	0x139B2820,
	0x139B2C08,
	0x139B2FF0,
	0x139B33D8,
	0x13A2C940,
	0x13A2CD28,
	0x13A2D110,
	0x13A2D4F8,
	0x13AA6A60,
	0x13AA6E48,
	0x13AA7618,
	0x13B20B80,
	0x13B20F68,
	0x13B21350,
	0x13B21738,
	0x13C14DC0,
	0x13C151A8,
	0x13C15590,
	0x13C15978,
	0x13C8EEE0,
	0x13C8F2C8,
	0x13C8F6B0,
	0x13C8FA98,
	0x13D09000,
	0x13D093E8,
	0x13D097D0,
	0x13D09BB8,
	0x13D83120,
	0x13D83508,
	0x13D838F0,
	0x13D83CD8,
	0x13DFD628,
	0x13EF1480,
	0x13EF1868,
	0x13EF2038,
	0x13F6B988,
	0x13F6C158,
	0x1405F7E0,
	0x1405FBC8,
	0x1405FFB0,
	0x14060398,
	0x140D9900,
	0x140D9CE8,
	0x140DA0D0,
	0x140DA4B8,
	0x14153A20,
	0x142C1D80,
	0x142C2168,
	0x142C2550,
	0x142C2938,
	0x143B5FC0,
	0x143B63A8,
	0x143B6790,
	0x143B6B78,
	0x144AA200,
	0x144AA5E8,
	0x144AA9D0,
	0x144AADB8,
	0x1459E440,
	0x1459E828,
	0x1459EC10,
	0x1459EFF8,
	0x14692680,
	0x14692A68,
	0x14692E50,
	0x14693238,
	0x1470C7A0,
	0x1470CB88,
	0x1470CF70,
	0x1470D358,
	0x147868C0,
	0x14786CA8,
	0x14787090,
	0x14787478,
	0x148009E0,
	0x14800DC8,
	0x148011B0,
	0x14801598,
	0x1487AB00,
	0x1487AEE8,
	0x1487B2D0,
	0x1487B6B8,
	0x148F4C20,
	0x148F5008,
	0x148F53F0,
	0x148F57D8,
	0x1496ED40,
	0x1496F128,
	0x1496F510,
	0x1496F8F8,
	0x149E8E60,
	0x149E9248,
	0x149E9630,
	0x149E9A18,
	0x14A63368,
	0x14A63750,
	0x14A63B38,
	0x14ADD0A0,
	0x14B571C0,
	0x14B575A8,
	0x14B57990,
	0x14B57D78,
	0x14BD12E0,
	0x14BD16C8,
	0x14BD1AB0,
	0x14BD1E98,
	0x14C4B400,
	0x14C4B7E8,
	0x14C4BBD0,
	0x14C4BFB8,
	0x14CC5520,
	0x14CC5908,
	0x14CC5CF0,
	0x14CC60D8,
	0x14D3F640,
	0x14D3FA28,
	0x14D3FE10,
	0x14D401F8,
	0x14DB9760,
	0x14DB9B48,
	0x14DB9F30,
	0x14DBA318,
	0x14E33880,
	0x14E33C68,
	0x14E34050,
	0x14E34438,
	0x14EAD9A0,
	0x14EADD88,
	0x14EAE170,
	0x14EAE558,
	0x14F27AC0,
	0x14F27EA8,
	0x14F28290,
	0x14F28678,
	0x14FA1BE0,
	0x1501BD00,
	0x1501C0E8,
	0x1501C4D0,
	0x1501C8B8,
	0x15095E20,
	0x1510FF40,
	0x15110328,
	0x15110710,
	0x15110AF8,
	0x15204180,
	0x15204568,
	0x15204950,
	0x15204D38,
	0x152F83C0,
	0x152F87A8,
	0x152F8B90,
	0x152F8F78,
	0x153EC600,
	0x153EC9E8,
	0x153ECDD0,
	0x153ED1B8,
	0x154E0C28,
	0x154E1010,
	0x154E13F8,
	0x155D4A80,
	0x155D4E68,
	0x155D5250,
	0x155D5638,
	0x156C8CC0,
	0x156C90A8,
	0x156C9490,
	0x156C9878,
	0x157BCF00,
	0x157BD2E8,
	0x157BD6D0,
	0x157BDAB8,
	0x158B1140,
	0x158B1528,
	0x158B1910,
	0x158B1CF8,
	0x159A5380,
	0x159A5768,
	0x159A5B50,
	0x159A5F38,
	0x15A995C0,
	0x15A999A8,
	0x15A99D90,
	0x15A9A178,
	0x15B8D800,
	0x15B8DBE8,
	0x15B8DFD0,
	0x15B8E3B8,
	0x15C81A40,
	0x15C81E28,
	0x15C82210,
	0x15C825F8,
	0x15D75C80,
	0x15D76068,
	0x15D76450,
	0x15D76838,
	0x15E69EC0,
	0x15E6A2A8,
	0x15E6A690,
	0x15E6AA78,
	0x15E6C5D0,
	0x15E6C9B8,
	0x15E6CDA0,
	0x15E6D188,
	0x15E6ECE0,
	0x15E6F0C8,
	0x15E6F4B0,
	0x15E6F898,
	0x15E713F0,
	0x15E73B00,
	0x15E76210,
	0x15E78920,
	0x15E7D740,
	0x15E7FE50,
	0x15E80238,
	0x15E80620,
	0x15E80A08,
	0x15E82560,
	0x15E84C70,
	0x15E87380,
	0x15E89A90,
	0x15E8C1A0,
	0x15E8E8B0,
	0x15E95DE0,
	0x15E961C8,
	0x15E965B0,
	0x15E96998,
	0x15E984F0,
	0x15E988D8,
	0x15E98CC0,
	0x15E990A8,
	0x15E9AC00,
	0x15E9AFE8,
	0x15E9B3D0,
	0x15E9B7B8,
	0x15E9D310,
	0x15E9D6F8,
	0x15E9DAE0,
	0x15E9DEC8,
	0x15E9FA20,
	0x15E9FE08,
	0x15EA01F0,
	0x15EA05D8,
	0x15EA2130,
	0x15EA2518,
	0x15EA2900,
	0x15EA2CE8,
	0x15EA4840,
	0x15EA4C28,
	0x15EA5010,
	0x15EA53F8,
	0x15EA6F50,
	0x15EA7338,
	0x15EA7720,
	0x15EA7B08,
	0x15EA9660,
	0x15EA9A48,
	0x15EA9E30,
	0x15EAA218,
	0x15EABD70,
	0x15EAC158,
	0x15EAC540,
	0x15EAC928,
	0x15EAE480,
	0x15EAE868,
	0x15EAEC50,
	0x15EAF038,
	0x15EB0B90,
	0x15EB0F78,
	0x15EB1360,
	0x15EB1748,
	0x15EB32A0,
	0x15EB3688,
	0x15EB3A70,
	0x15EB3E58,
	0x15EB59B0,
	0x15EB5D98,
	0x15EB6180,
	0x15EB6568,
	0x15EB80C0,
	0x15EB84A8,
	0x15EB8890,
	0x15EB8C78,
	0x15EBA7D0,
	0x15EBCEE0,
	0x15EBD6B0,
	0x15EBF5F0,
	0x15EC1D00,
	0x15EC20E8,
	0x15EC24D0,
	0x15EC28B8,
	0x15EC4410,
	0x15EC6B20,
	0x15EC9230,
	0x15EC9618,
	0x15EC9A00,
	0x15EC9DE8,
	0x15ECB940,
	0x15ECBD28,
	0x15ECC110,
	0x15ECC4F8,
	0x15ECE050,
	0x15ECE438,
	0x15ECE820,
	0x15ECEC08,
	0x15ED0760,
	0x15ED0B48,
	0x15ED0F30,
	0x15ED1318,
	0x15ED2E70,
	0x15ED3258,
	0x15ED3640,
	0x15ED3A28,
	0x15ED5580,
	0x15ED5968,
	0x15ED5D50,
	0x15ED6138,
	0x15ED7C90,
	0x15ED8078,
	0x15ED8460,
	0x15ED8848,
	0x15EDA3A0,
	0x15EDCAB0,
	0x15EDF1C0,
	0x15EE18D0,
	0x15EE3FE0,
	0x15EE66F0,
	0x15EE8E00,
	0x15EEB510,
	0x15EEDC20,
	0x15EF0330,
	0x15EF2A40,
	0x15EF5150,
	0x15EF7860,
	0x15EF9F70,
	0x15EFC680,
	0x15EFED90,
	0x15F014A0,
	0x15F03BB0,
	0x15F062C0,
	0x15F089D0,
	0x15F0B0E0,
	0x15F0D7F0,
	0x15F0FF00,
	0x15F12610,
	0x15F129F8,
	0x15F12DE0,
	0x15F131C8,
	0x15F14D20,
	0x15F15108,
	0x15F154F0,
	0x15F158D8,
	0x15F17430,
	0x15F17818,
	0x15F17C00,
	0x15F17FE8,
	0x15F19B40,
	0x15F19F28,
	0x15F1A310,
	0x15F1A6F8,
	0x15F1C250,
	0x15F1C638,
	0x15F1CA20,
	0x15F1CE08,
	0x15F1E960,
	0x15F1ED48,
	0x15F1F130,
	0x15F1F518,
	0x15F21070,
	0x15F21458,
	0x15F21840,
	0x15F21C28,
	0x15F23780,
	0x15F23B68,
	0x15F23F50,
	0x15F24338,
	0x15F25E90,
	0x15F26278,
	0x15F26660,
	0x15F26A48,
	0x15F285A0,
	0x15F28988,
	0x15F28D70,
	0x15F29158,
	0x15F2ACB0,
	0x15F2B098,
	0x15F2B480,
	0x15F2B868,
	0x15F2D3C0,
	0x15F2D7A8,
	0x15F2DB90,
	0x15F2DF78,
	0x15F2FAD0,
	0x15F2FEB8,
	0x15F302A0,
	0x15F30688,
	0x15F321E0,
	0x15F325C8,
	0x15F329B0,
	0x15F32D98,
	0x15F348F0,
	0x15F34CD8,
	0x15F350C0,
	0x15F354A8,
	0x15F37000,
	0x15F373E8,
	0x15F377D0,
	0x15F37BB8,
	0x15F39710,
	0x15F39AF8,
	0x15F39EE0,
	0x15F3A2C8,
	0x15F3BE20,
	0x15F3C208,
	0x15F3C5F0,
	0x15F3C9D8,
	0x15F3E530,
	0x15F40C40,
	0x15F41028,
	0x15F41410,
	0x15F417F8,
	0x15F43350,
	0x15F43738,
	0x15F43B20,
	0x15F43F08,
	0x109ED810,
	0x109EFF20,
	0x109F2630,
	0x109F4D40,
	0x109F7450,
	0x109F9B60,
	0x15EE3FE0,
	0x109FC270,
	0x109EDBF8,
	0x109F0308,
	0x109F2A18,
	0x109F5128,
	0x109F7838,
	0x109F9F48,
	0x109FED68,
	0x15EE43C8,
	0x109FC658,
	0x109EE3C8,
	0x109F0AD8,
	0x109F31E8,
	0x109F58F8,
	0x109F8008,
	0x109FA718,
	0x109FF538,
	0x15EE4B98,
	0x109FCE28,
	0x109EDFE0,
	0x109F06F0,
	0x109F2E00,
	0x109F5510,
	0x109F7C20,
	0x109FA330,
	0x109FF150,
	0x15EE47B0,
	0x10A01090,
	0x10A037A0,
	0x10A05EB0,
	0x10A085C0,
	0x10A0ACD0,
	0x109FED68,
	0x10A01478,
	0x10A037A0,
	0x10A03B88,
	0x10A06298,
	0x10A089A8,
	0x10A0B0B8,
	0x109FF150,
	0x10A01860,
	0x10A03F70,
	0x10A06680,
	0x10A08D90,
	0x10A09178,
	0x10A0B4A0,
	0x109FF538,
	0x10A01C48,
	0x10A04358,
	0x10A06A68,
	0x10A09178,
	0x10A0B888,
	//CONSUMABLES
	0x40001580,
	0x40001581,
	0x40001582,
	0x400000F0,
	0x400000F1,
	0x400000F2,
	0x40000105,
	0x4000010E,
	0x4000010F,
	0x40000110,
	0x40000112,
	0x40000113,
	0x40000114,
	0x40000118,
	0x40000124,
	0x40000125,
	0x40000126,
	0x40000128,
	0x40000129,
	0x4000012B,
	0x4000012C,
	0x4000012E,
	0x4000012F,
	0x40000130,
	0x40000136,
	0x40000137,
	0x40000140,
	0x40000141,
	0x40000142,
	0x40000143,
	0x40000144,
	0x40000145,
	0x40000146,
	0x4000014A,
	0x4000014B,
	0x4000014E,
	0x4000014F,
	0x40000150,
	0x40000154,
	0x40000155,
	0x40000157,
	0x40000158,
	0x4000015E,
	0x40000172,
	0x40000190,
	0x40000191,
	0x40000192,
	0x40000193,
	0x40000194,
	0x40000195,
	0x40000196,
	0x40000197,
	0x40000198,
	0x40000199,
	0x4000019A,
	0x4000019B,
	0x4000019C,
	0x4000019D,
	0x4000019E,
	0x4000019F,
	0x400001A0,
	0x400001A1,
	0x400001A2,
	0x400001A3,
	0x400001A4,
	0x400001A5,
	0x400001B8,
	0x400001C3,
	0x400001C4,
	0x400001C5,
	0x400001C6,
	0x400001C7,
	0x400001C8,
	0x400001C9,
	0x400001CA,
	0x400001CB,
	0x400001CC,
	0x400001CD,
	0x400001F4,
	0x400002BC,
	0x400002BD,
	0x400002BE,
	0x400002BF,
	0x400002C8,
	0x400002C9,
	0x400002CA,
	0x400002CB,
	0x400002CD,
	0x400002CE,
	0x400002CF,
	0x400002D0,
	0x400002D1,
	0x400002D2,
	0x400002D3,
	0x400002D4,
	0x400002D5,
	0x400002D6,
	0x400002D7,
	0x400002D8,
	0x400002D9,
	0x400002DB,
	0x400002DC,
	0x400002DD,
	0x400002E3,
	0x400002E7,
	0x400002E8,
	0x400002E9,
	0x400002EA,
	0x400002EB,
	0x400002EC,
	0x400002ED,
	0x400002EE,
	0x400002EF,
	0x400002F0,
	0x400002F4,
	0x400002F1,
	0x400002F2,
	0x400002F3,
	0x40000320,
	0x40000321,
	0x40000322,
	0x40000323,
	0x40000324,
	0x40000325,
	0x40000326,
	0x40000879,
	0x40000BBC,
	0x40000BBD,
	0x40000BCA,
	0x40000BCC,
	0x40000BCD,
	0x40000BCE,
	0x40000BCF,
	0x40000BD0,
	0x40000FB1,
	0x40000FB2,
	0x40000FB3,
	0x40001068,
	0x40001069,
	0x4000106A,
	0x4000106B,
	0x4000106C,
	0x4000106D,
	0x4000106E,
	0x40000BCB,
	0x40000149,
	0x40000BBB,
	0x40000148,
	//SPELLBOUND ITEMS
	0x40001518,
	0x40001519,
	0x4000151A,
	0x4000151B,
	0x4000151C,
	0x40001522,
	0x4000152C,
	0x4000152D,
	0x4000152E,
	0x4000152F,
	0x40001530,
	0x40001531,
	0x40001532,
	0x40001533,
	0x40001534,
	0x40001540,
	0x4000154A,
	0x40001554,
	0x4000155E,
	0x40001568,
	0x40001569,
	0x4000156A,
	0x4000156B,
	0x4000156C,
	0x4000156D,
	0x4000156E,
	0x4000156F,
	0x40001570,
	0x40001572,
	0x40001573,
	0x40001004,
	0x40001005,
	0x4000100E,
	0x4000100F,
	0x40001518,
	0x40001519,
	0x4000151A,
	0x4000151B,
	0x4000151C,
	0x4000151D,
	0x40001522,
	0x4000152C,
	0x4000152D,
	0x4000152E,
	0x4000152F,
	0x40001530,
	0x40001531,
	0x40001532,
	0x40001533,
	0x40001534,
	0x40001535,
	0x40001540,
	0x4000154A,
	0x40001554,
	0x4000155E,
	0x40001568,
	0x40001569,
	0x4000156A,
	0x4000156B,
	0x4000156C,
	0x4000156D,
	0x4000156E,
	0x4000156F,
	0x40001570,
	0x40001572,
	0x40001573,
	0x4000157C,
	0x4000157D,
	0x4000157E,
	0x4000157F,
	0x40001580,
	0x40001581,
	0x40001582,
	//MISC
	0x4000157C,
	0x4000157D,
	0x4000157E,
	0x4000157F,
	0x40004E55,
	0x40004E56,
	0x4000017F,
	0x40000180,

	//MISC
	0x4000017C,
	0x40000873,
	//KEY ITEMS
	0x40000186,
	0x400001EA,
	0x400007D0,
	0x400007D1,
	0x400007D2,
	0x400007D4,
	0x400007D5,
	0x400007D6,
	0x400007D7,
	0x400007D8,
	0x400007D9,
	0x400007DA,
	0x400007DC,
	0x400007DE,
	0x40000811,
	0x40000834,
	0x40000836,
	0x40000837,
	0x40000838,
	0x40000839,
	0x4000083A,
	0x40000845,
	0x40000846,
	0x40000847,
	0x40000848,
	0x40000849,
	0x4000084B,
	0x4000084C,
	0x4000084D,
	0x4000084E,
	0x4000084F,
	0x40000850,
	0x40000851,
	0x40000852,
	0x40000853,
	0x40000854,
	0x40000855,
	0x40000856,
	0x40000859,
	0x4000085A,
	0x4000085B,
	0x4000085C,
	0x4000085E,
	0x40000860,
	0x40000861,
	0x4000086B,
	0x4000086C,
	0x4000086E,
	0x4000087D,
	0x4000087E,
	0x4000087F,
	0x40000880,
	0x40000881,
	0x40000882,
	0x400007E0,
	//ACCESSORIES
	0x20000066,
	0x2000007D,
	0x20000080,
	0x2000008B,
	0x2000008C,
	0x2000008F,
	0x20000095,
	0x20004E20,
	0x20004E2A,
	0x20004E34,
	0x20004E3E,
	0x20004E48,
	0x20004E52,
	0x20004E5C,
	0x20004E66,
	0x20004E70,
	0x20004E7A,
	0x20004E84,
	0x20004E8E,
	0x20004E98,
	0x20004EA2,
	0x20004EAC,
	0x20004EB6,
	0x20004EC0,
	0x20004ECA,
	0x20004ED4,
	0x20004EDE,
	0x20004EE8,
	0x20004EF2,
	0x20004F06,
	0x20004F10,
	0x20004F1A,
	0x20004F2E,
	0x20004F38,
	0x20004F42,
	0x20004F4C,
	0x20004F56,
	0x20004F60,
	0x20004F6A,
	0x20004F74,
	0x20004F7E,
	0x20004F88,
	0x20004F92,
	0x20004F9C,
	0x20004FA6,
	0x20004FB0,
	0x20004FBA,
	0x20004FC4,
	0x20004FCE,
	0x20004FE2,
	0x20004FEC,
	0x20004FF6,
	0x20005000,
	0x2000500A,
	0x20005014,
	0x2000501E,
	0x20005028,
	0x2000503C,
	0x20005046,
	0x20005064,
	0x2000506E,
	0x20005078,
	0x20005082,
	0x2000508C,
	0x20005096,
	0x200050B4,
	0x200050BE,
	0x200050DC,
	0x200050E6,
	0x200050F0,
	0x200050FA,
	0x20005104,
	0x2000510E,
	0x20005136,
	0x2000515E,
	0x20005208,
	0x20007530,
	0x2000753A,
	0x20007544,
	0x20007558,
	0x20007562,
	0x2000756C,
	0x20007576,
	0x20007580,
	0x2000758A,
	0x200075DA,
	0x200075E4,
	0x200075EE,
	0x200075F8,
	0x20007602,
	0x2000767A,
	0x20007684,
	0x2000768E,
	0x20007698,
	0x200076A2,
	0x200076AC,
	0x200076B6,
	0x200076C0,
	0x200076CA,
	0x200076D4,
	0x200076DE,
	0x200076F2,
	0x20007724,
	0x2000772E,
	0x20007738,
	0x20007742,
	0x2000774C,
	0x20007760,
	0x2000776A,
	0x200077BA,
	0x200077C4,
	0x200077CE,
	0x200077D8,
	0x200077E2,
	0x200077EC,
	0x20007850,
	0x20007878,
	0x20007896,
	0x200078A0,
	0x200078AA,
	0x200078B4,
	0x200078BE,
	0x200078C8,
	0x200078D2,
	0x200078DC,
	0x200078E6,
	0x200078F0,
	0x200078FA,
	0x20007904,
	0x2000790E,
	0x20007918,
	0x20007922,
	0x20007936,
	0x20007940,
	0x2000794A,
	0x20007954,
	0x2000795E,
	0x2000797C,
	0x20007986,
	0x20007990,
	0x2000799A,
	0x20007E36,
	0x20007968,
	0x200079E0,
	0x20007A44,
	0x20007AA8,
	0x20007B0C,
	0x20007B70,
	0x20007BD4,
	0x20007C38,
	0x20007C9C,
	0x20007D00,
	0x20007D64,
	0x20007DC8,
	0x20007E2C,
	//SHIELDS
	0x01312D00,
	0x01315410,
	0x0131A230,
	0x0131C940,
	0x01323E70,
	0x01326580,
	0x0132DAB0,
	0x013301C0,
	0x013328D0,
	0x013376F0,
	0x01339E00,
	0x0133C510,
	0x0133EC20,
	0x01341330,
	0x01343A40,
	0x01346150,
	0x01348860,
	0x0134AF70,
	0x0134D680,
	0x0134FD90,
	0x013524A0,
	0x01354BB0,
	0x013572C0,
	0x013599D0,
	0x0135C0E0,
	0x0135E7F0,
	0x01409650,
	0x01410B80,
	0x014159A0,
	0x014180B0,
	0x0141F5E0,
	0x01421CF0,
	0x01424400,
	0x01426B10,
	0x01429220,
	0x0142B930,
	0x0142E040,
	0x01430750,
	0x01432E60,
	0x01435570,
	0x01437C80,
	0x0143A390,
	0x0143CAA0,
	0x0143F1B0,
	0x014418C0,
	0x01443FD0,
	0x014466E0,
	0x01448DF0,
	0x0144B500,
	0x0144DC10,
	0x01450320,
	0x01452A30,
	0x014FD890,
	0x014FFFA0,
	0x01504DC0,
	0x015074D0,
	0x01509BE0,
	0x0150C2F0,
	0x0150EA00,
	0x01511110,
	0x01513820,
	0x01515F30,
	0x01518640,
	0x0151AD50,
	0x0151D460,
	0x0151FB70,
	0x01522280,
	0x01524990,
	0x015270A0,
	0x015297B0,
	0x0152BEC0,
	0x0152E5D0,
	0x01530CE0,
	0x015333F0,
	0x01535B00,
	0x01538210,
	0x0153A920,
	0x0153D030,
	0x0153F740,
	0x01541E50,
	0x01544560,
	0x01546C70,
	0x01549380,
	//SORCERIES
	0x40124F80,
	0x40127690,
	0x4013D620,
	0x4013DA08,
	0x4013DDF0,
	0x4013E1D8,
	0x4013E5C0,
	0x4013E9A8,
	0x4013ED90,
	0x4013F178,
	0x4013FD30,
	0x40140500,
	0x401408E8,
	0x40144B50,
	0x40144F38,
	0x40147260,
	0x40147648,
	0x40149970,
	0x4014A528,
	0x4014A910,
	0x4014ACF8,
	0x4014B0E0,
	0x4014E790,
	0x4014EF60,
	0x4014F348,
	0x4014F730,
	0x4014FB18,
	0x4014FF00,
	0x4018B820,
	0x40193138,
	0x401A8CE0,
	0x401A90C8,
	0x401B7740,
	0x401B7B28,
	0x401B7F10,
	0x401B82F8,
	0x401B8AC8,
	0x401B9298,
		0x401B82F8,
		0x401B8AC8,
		0x401B9298,
		0x401B9680,
		0x401B96E4,
		0x401B96E4,
		0x401B97AC,
		0x401B9810,
		0x401B9874,
		0x401B9A68,
		0x401B9E50,
		0x401BA238,
		0x401BA620,
	//PYROMANCIES
	0x40249F00,
	0x4024A6D0,
	0x4024AAB8,
	0x4024B288,
	0x4024C9F8,
	0x4024ED20,
	0x4024F108,
	0x4024F4F0,
	0x40251430,
	0x40251818,
	0x402527B8,
	0x40252BA0,
	0x40253B40,
	0x40256250,
	0x40256638,
	0x40256A20,
	0x402575D8,
	0x402579C0,
	0x40257DA8,
	0x40258190,
	0x4025B070,
	0x4025B07A,
	0x4027FA60,
	0x40282170,
	0x40284880,
	0x40286F90,
	0x402896A0,
	0x402936C8,
	0x40293AB0,
	0x402959F0,
	0x40295DD8,
	0x402961C0,
	0x402965A8,
	0x40296990,
	0x40298100,
	0x402579C0,
		0x40293AB0,
		0x402959F0,
		0x40295DD8,
		0x402961C0,
		0x402965A8,
		0x40296990,
		0x40298100,
		0x402984E8,
		0x402988D0,
		0x402988D0,
		0x402990A0,
		0x402990A0,
		0x40299870,
		0x40299C58,
		0x4029A810,
		0x4029ABF8,
		0x4029ABF8,
		0x4029ABF8,
		0x4029B7B0,
		0x4029BB98,
		0x4029BF80,
		0x4029C368,
		0x4029C750,
		0x4029FA18,
		0x4029FE00,
		0x402A01E8,
		0x402A05D0,
		0x402A09B8,
		0x402A0DA0,
	//MIRACLES
	0x403540D0,
	0x403567E0,
	0x40356BC8,
	0x40356FB0,
	0x40357398,
	0x40357780,
	0x40357B68,
	0x40358338,
	0x40358720,
	0x40358B08,
	0x4035B600,
	0x4035B9E8,
	0x4035DD10,
	0x4035E0F8,
	0x4035E4E0,
	0x40360420,
	0x40362B30,
	0x40362F18,
	0x40363300,
	0x403636E8,
	0x403642A0,
	0x40364688,
	0x40365240,
	0x40365DF8,
	0x4036C770,
	0x4036CB58,
	0x40389C30,
	0x4038C340,
	0x40395F80,
	0x403A0390,
	0x403A0778,
	0x403A0F48,
	0x403A2AA0,
	0x403A2E88,
	0x403A3270,
	0x403A3658,
	0x403A3A40,
	0x403A4210,
	0x403A45F8,
	0x403A49E0,
	0x403A4DC8,
		0x403A0390,
		0x403A0778,
		0x403A0F48,
		0x403A2AA0,
		0x403A2E88,
		0x403A3270,
		0x403A3658,
		0x403A3A40,
		0x403A4210,
		0x403A45F8,
		0x403A49E0,
		0x403A4DC8,
		0x403A51B0,
		0x403A51B0,
		0x403A5980,
		0x403A5D68,
		0x403A6150,
		0x403A6538,
		0x403A6920,
		0x403A6D08,
		0x403A70F0,
		0x403A74D8,
	//HEXES
	0x403E8FA0,
	0x403E9388,
	0x403E9770,
	0x403E9B58,
	0x403E9F40,
	0x403EA328,
	0x403EA710,
	0x403EAAF8,
	0x403EAEE0,
	0x403EB2C8,
	0x403EB6B0,
	0x403EBA98,
	0x403EBE80,
	0x403EC268,
	0x403EC650,
	0x403ECA38,
	0x403ECE20,
	0x40401A28,
	0x40401E10,
	0x404021F8,
	0x404025E0,
	0x404029C8,
	0x40402DB0,
	0x40403198,
	0x40403580,
	0x40403968,
	0x40403D50,
	0x4041A0C8,
	0x4041A4B0,
	0x4041A898,
	0x4041AC80,
	0x4041B068,
	0x4041B450,
	0x4041B838,
	0x4041BC20,
	0x4041C008,
	0x4041C3F0,
	0x4041C7D8,
	0x4041CBC0,
	0x4041CFA8,
		0x4041D390,
		0x4041D778,
		0x4041DB60,
		0x4041DF48,
		0x4041E330,
		0x4041E718,
		0x4041EB00,
	//UTILITY
	0x40000064,
	0x40000065,
	0x40000066,
	0x40000067,
	0x4000006C,
	0x4000006F,
	0x40000073,
	0x40000075,
	0x40000077,
	0x4000013B,
	0x4000015F,
	0x40000173,
	0x40000178,
	0x40000181,
	0x40000208,
	0x40000209,
	0x4000020A,
	0x4000020B,
	0x4000020C,
	0x4000028A,
	0x4000028B,
	0x40000385,
	0x400003E8,
	0x400003E9,
	0x400003EA,
	0x400003EB,
	0x40000424,
	0x4000042E,
	0x40000438,
	0x40000442,
	0x400004E2,
	0x4000085D,
	0x4000085F,
	0x4000086F,
	0x40000898,
	0x40000899,
	0x40000FA0,
	0x40000FA1,
	0x40000FA2,
	0x40000FA3,
	0x40000FA4,
	0x40000FA5,
	0x40000FA6,
	0x40000FA7,
	0x40000FA8,
	0x40000FA9,
	0x40000FAA,
	0x40000FAB,
	0x40000FAC,
	0x40000FAD,
	0x40001004,
	0x40001005,
	//SPELL TOOLS
	0x01038D50,
	0x0103B460,
	0x0103DB70,
	0x01040280,
	0x01042990,
	0x010450A0,
	0x010477B0,
	0x01049EC0,
	0x0104C5D0,
	0x0104ECE0,
	0x010513F0,
	0x01053B00,
	0x01056210,
	0x0112A880,
	0x0112CF90,
	0x0112F6A0,
	0x01131DB0,
	0x011344C0,
	0x01136BD0,
	0x011392E0,
	0x0113B9F0,
	0x0113E100,
	0x01140810,
	0x01142F20,
	0x01145630,
	0x01147D40,
	0x0121EAC0,
	0x012211D0,
	0x012238E0,
	0x01225FF0,
	0x01228700,
	0x0122AE10,
	0x0122D520,
	0x0122FC30,
	0x01232340,
	0x01234A50,
	0x01237160,
	0x01239870,
	0x0123BF80,
	0x0123E690,
	0x01240DA0,
	0x012434B0,
	0x012E1FC0,
	0x012E46D0,
	0x012E6DE0,
	0x012E94F0,
	0x012FA660,
	//WEAPONS
	0x000F4240,
	0x000F6950,
	0x000F9060,
	0x000FB770,
	0x000FDE80,
	0x00102CA0,
	0x001053B0,
	0x00107AC0,
	0x0010A1D0,
	0x00111700,
	0x00116520,
	0x00118C30,
	0x0011B340,
	0x001E8480,
	0x001EAB90,
	0x001ED2A0,
	0x001EF9B0,
	0x001F6EE0,
	0x00203230,
	0x00205940,
	0x0020A760,
	0x002143A0,
	0x002191C0,
	0x0021B8D0,
	0x0021DFE0,
	0x002206F0,
	0x00222E00,
	0x00225510,
	0x00227C20,
	0x0022A330,
	0x002DC6C0,
	0x002DEDD0,
	0x002E14E0,
	0x002E3BF0,
	0x002E6300,
	0x002E8A10,
	0x002EB120,
	0x003D3010,
	0x003D7E30,
	0x003DA540,
	0x003DCC50,
	0x003DF360,
	0x003E1A70,
	0x003E4180,
	0x003E6890,
	0x003E8FA0,
	0x003EB6B0,
	0x003EDDC0,
	0x004C4B40,
	0x004C7250,
	0x004C9960,
	0x004CC070,
	0x004CE780,
	0x004D0E90,
	0x004D35A0,
	0x005B8D80,
	0x005BB490,
	0x005BDBA0,
	0x005C29C0,
	0x005C50D0,
	0x005C77E0,
	0x005C9EF0,
	0x005CC600,
	0x005D1420,
	0x005D8950,
	0x005DB060,
	0x005DD770,
	0x005E2590,
	0x005E4CA0,
	0x005E73B0,
	0x005E9AC0,
	0x005F0FF0,
	0x005F3700,
	0x005F5E10,
	0x005F8520,
	0x005FAC30,
	0x005FD340,
	0x005FFA50,
	0x00602160,
	0x00604870,
	0x00606F80,
	0x00609690,
	0x0060BDA0,
	0x0060E4B0,
	0x00610BC0,
	0x006132D0,
	0x006159E0,
	0x006ACFC0,
	0x006AF6D0,
	0x006B1DE0,
	0x006B6C00,
	0x006B9310,
	0x006BE130,
	0x006C0840,
	0x006C2F50,
	0x006C5660,
	0x006C7D70,
	0x006CA480,
	0x006CCB90,
	0x006D19B0,
	0x006D67D0,
	0x006D8EE0,
	0x007A1200,
	0x007A3910,
	0x007A6020,
	0x007A8730,
	0x007AFC60,
	0x007B4A80,
	0x007BBFB0,
	0x007C8300,
	0x007CAA10,
	0x007CD120,
	0x007CF830,
	0x007D4650,
	0x007D9470,
	0x007DBB80,
	0x007DE290,
	0x007E09A0,
	0x007E30B0,
	0x007E57C0,
	0x007E7ED0,
	0x007EA5E0,
	0x007ECCF0,
	0x007EF400,
	0x00895440,
	0x00897B50,
	0x0089C970,
	0x008A8CC0,
	0x008AB3D0,
	0x008ADAE0,
	0x008B01F0,
	0x008B2900,
	0x008B5010,
	0x008B7720,
	0x008BC540,
	0x008BEC50,
	0x008C1360,
	0x008C3A70,
	0x008C6180,
	0x008CAFA0,
	0x008CD6B0,
	0x008CFDC0,
	0x008D24D0,
	0x008D4BE0,
	0x00989680,
	0x0098BD90,
	0x0098E4A0,
	0x00990BB0,
	0x009959D0,
	0x0099A7F0,
	0x0099CF00,
	0x0099F610,
	0x009A1D20,
	0x009A4430,
	0x009A6B40,
	0x009AB960,
	0x009AE070,
	0x009B2E90,
	0x009B55A0,
	0x00A7D8C0,
	0x00A7FFD0,
	0x00A826E0,
	0x00A84DF0,
	0x00A87500,
	0x00A89C10,
	0x00A8EA30,
	0x00B71B00,
	0x00B7B740,
	0x00B7DE50,
	0x00B80560,
	0x00B8C8B0,
	0x00B8EFC0,
	0x00B916D0,
	0x00B93DE0,
	0x00B964F0,
	0x00B98C00,
	0x00B9B310,
	0x00B9DA20,
	0x00BA0130,
	0x00D5C690,
	0x00D5EDA0,
	0x00D614B0,
	0x00D63BC0,
	0x00D662D0,
	0x00D689E0,
	0x00D6B0F0,
	0x00D6FF10,
	0x00D72620,
	0x00D74D30,
	0x00D77440,
	0x00D79B50,
	0x00D7C260,
	0x00D7E970,
	0x00D83790,
	0x00D85EA0,
	0x00D885B0,
	0x00D8ACC0,
	0x00D8D3D0,
	0x00D8FAE0,
	0x00D85EA0,
	0x00D921F0,
	0x00DBBA00,
	0x00DBE110,
	0x00DC0820,
	0x00DC2F30,
	0x00DC5640,
	0x00DC7D50,
	0x00DCA460,
	0x00DCCB70,
	0x00F42400,
	0x00F47220,
	0x00F49930,
	0x00F4C040,
	0x00F4E750,
	0x00F50E60,
	0x00F53570,
	0x00F55C80,
	0x00F58390,
	0x00F5AAA0,
	0x00F5F8C0,
	0x00F61FD0,
	0x00F646E0,
	0x00F66DF0,
	0x00F69500,
	0x00F6BC10,
	0x015EF3C0,
	0x015F1AD0,
	0x01C9EA90,
	0x01CA11A0,
	0x01CA38B0,
	0x01CA5FC0,
	0x01CA86D0,
	0x01CAADE0,
	0x01CAD4F0,
	0x01CAFC00,
	0x01CB2310,
	0x01CB4A20,
	0x01CB7130,
	0x01CBBF50,
	0x01CBE660,
	0x01CC5B90,
	0x01CC82A0,
	0x01CCD0C0,
	0x01CD1EE0,
	0x01CD6D00,
	0x01CD9410,
	0x01CDBB20,
	0x01CDE230,
	0x01CE5760,
	0x01CE7E70,
	0x01CEA580,
	0x01CECC90,
	0x01CEF3A0,
	0x01CF1AB0,
	0x01CF41C0,
	0x01CF68D0,
	0x01CF8FE0,
	0x01CFB6F0,
	0x01CFDE00,
	0x01D00510,
	0x01D02C20,
	0x01D05330,
	0x01D07A40,
	0x01D0A150,
	0x01D0C860,
	0x01D0EF70,
	0x01D13D90,
	0x01D164A0,
	0x01D18BB0,
	0x01D1B2C0,
	0x01D200E0,
	0x01D227F0,
	0x01D24F00,
	0x01D27610,
	0x01D29D20,
	0x01D2C430,
	0x01D31250,
	0x01D33960,
	0x01D36070,
	0x01D38780,
	0x01D3AE90,
	0x01D3D5A0,
	0x01D3FCB0,
	0x01D423C0,
	0x01D44AD0,
	0x01D471E0,
	0x01D498F0,
	0x01D4C000,
	0x01D50E20,
	0x01D55C40,
	0x01D58350,
	0x01D5AA60,
	0x01D5D170,
	0x01D5F880,
	0x01D646A0,
	0x01D66DB0,
	0x01D694C0,
	0x01D6BBD0,
	0x01D6E2E0,
	0x01D709F0,
	0x01D73100,
	0x01D75810,
	0x01D77F20,
	0x01D7F450,
	0x01D84270,
	0x01D86980,
	0x01D89090,
	0x01D8B7A0,
	0x01D8DEB0,
	0x01D92CD0,
	0x01D953E0,
	0x01D97AF0,
	0x01D9A200,
	0x01D9C910,
	0x01D9F020,
	0x01DA1730,
	0x01DA3E40,
	0x01DA6550,
	0x01DA8C60,
	0x01DAB370,
	0x01DADA80,
	0x01DB0190,
	0x01DB28A0,
	0x01DB76C0,
	0x01DB9DD0,
	0x01DBC4E0,
	0x01DBEBF0,
	0x01DC1300,
	0x01DC3A10,
	0x01DC6120,
	0x01DC8830,
	0x01DCAF40,
	0x01DCD650,
	0x01DCFD60,
	0x01DD2470,
	0x01DD4B80,
	0x01DD7290,
	0x01DD99A0,
	0x01DDC0B0,
	0x01DDE7C0,
	0x01DE0ED0,
	0x01DE35E0,
	0x01DE5CF0,
	0x01DEAB10,
	0x01DED220,
	0x01DEF930,
	0x01DF2040,
	0x01DF4750,
	0x01DF6E60,
	0x01DF9570,
	0x01DFBC80,
	0x01DFE390,
	0x01E00AA0,
	0x01E031B0,
	0x01E058C0,
	0x01E07FD0,
	0x01E0A6E0,
	0x01E0CDF0,
	0x01E0F500,
	0x01E11C10,
	0x01E14320,
	0x01E16A30,
	0x01E19140,
	0x01E1B850,
	0x01E1DF60,
	0x01E20670,
	0x01E22D80,
	0x01E25490,
	0x01E27BA0,
	0x01E2A2B0,
	0x01E2C9C0,
	0x01E2F0D0,
	0x01E317E0,
	0x01E33EF0,
	0x01E36600,
	0x01E38D10,
	0x01E3B420,
	0x01E3DB30,
	0x01E40240,
	0x01E42950,
	//COVENANTS
	0x20002710,
	0x20002724,
	0x2000272E,
	0x20002738,
	0x20002742,
	0x2000274C,
	0x20002756,
	0x20002760,
	0x2000276A,
	0x20002774,
	0x2000277E,
	0x20002788,
	0x20002792,
	0x2000279C,
	0x200027A6,
	0x200027B0,
	0x200027BA,
	0x200027C4,
	//BOONS
	0x4000A028,
	0x4000A029,
	0x4000A02A,
	0x4000A02B,
	0x4000A02C,
	0x4000A02D,
	0x4000A02E,
	0x4000A02F,
	0x4000A030,
	0x4000A031,
	0x4000A032,
	0x4000A033,
	0x4000A034,
	0x4000A035,
	0x4000A036,
	0x4000A037,
	0x4000A038,
	0x4000A039,
	0x4000A03A,
	0x4000A03B,
	0x4000A03C,
	0x4000A03D,
	0x4000A03E,
	0x4000A03F,
	0x4000A040,
	0x4000A041,
	0x4000A042,
	0x4000A043,
	0x4000A044,
	0x4000A045,
	0x4000A046,
	0x4000A047,
	0x4000A048,
	0x4000A049,
	//MALUSES
	0x4000A410,
	0x4000A411,
	0x4000A412,
	0x4000A413,
	0x4000A414,
	0x4000A415,
	0x4000A416,
	0x4000A417,
	0x4000A418,
	0x4000A419,
	0x4000A41A,
	0x4000A41B,
	0x4000A41C,
	0x4000A41D,
	0x4000A41E,
	0x4000A41F,
	0x4000A420,
	0x4000A421,
	0x4000A422,
	0x4000A423,
	0x4000A424,
	0x4000A425,
	0x4000A426,
	0x4000A427,
	0x4000A428,
	0x4000A429,
	0x4000A42A,
	0x4000A42B,
	0x4000A42C,
	0x4000A42D,
	0x4000A42E,
	0x4000A42F,
	0x4000A430,
	0x4000A431,
};