#include "Core.h"

extern CCore* Core;
extern CAutoEquip* AutoEquip;
extern SCore* CoreStruct;

DWORD dRingSlotSelect = 0x11;
DWORD pHelmetList[238];
DWORD pBodyList[194];
DWORD pHandsList[181];
DWORD pLegsList[182];

VOID fAutoEquip(UINT_PTR pItemBuffer, DWORD64 pItemData, DWORD64 qReturnAddress) {
	if (*(int*)(pItemData) >= 0) AutoEquip->AutoEquipItem(pItemBuffer, qReturnAddress);
	return;
};

VOID CAutoEquip::AutoEquipItem(UINT_PTR pItemBuffer, DWORD64 qReturnAddress) {

	SEquipBuffer pEquipBuffer;
	DWORD dItemAmount = 0;
	DWORD dItemID = 0;
	DWORD dItemQuantity = 0;
	DWORD dItemDurability = 0;

	if (!pItemBuffer) {
		Core->Panic("Null buffer!", "...\\Source\\AutoEquip\\AutoEquip.cpp", FE_NullPtr, 1);
		int3
	};

	dItemAmount = *(int*)pItemBuffer;
	pItemBuffer += 4;

	if (dItemAmount > 6) {
		Core->Panic("Too many items!", "...\\Source\\AutoEquip\\AutoEquip.cpp", FE_AmountTooHigh, 1);
		int3
	};
	
	while (dItemAmount) {

		dItemID = *(int*)(pItemBuffer);
		dItemQuantity = *(int*)(pItemBuffer + 0x04);
		dItemDurability = *(int*)(pItemBuffer + 0x08);

		if (SortItem(dItemID, &pEquipBuffer)) {
			if (!AutoEquip->EquipItem) {
				Core->Panic("Bad function call", "...\\Source\\AutoEquip\\AutoEquip.cpp", FE_BadFunc, 1);
				int3
			};
			LockUnlockEquipSlots(1);
			AutoEquip->EquipItem(pEquipBuffer.dEquipSlot, &pEquipBuffer);
		};
	
		dItemAmount--;
		pItemBuffer += 0x0C;
	};

	return;
};

BOOL CAutoEquip::SortItem(DWORD dItemID, SEquipBuffer* E) {

	char pBuffer[MAX_PATH];
	DWORD dItemType = 0;
	DWORD dEquipSlot = 0;

	dItemType = (dItemID >> 0x1C);

	switch (dItemType) {
	case(ItemType_Weapon): {
		if ((dItemID >> 0x10) == 6) return false; //Don't equip ammo
		if ((dItemID & 0xFF000000) << 4 != 0x10000000) dEquipSlot = 1; //If these conditions are met, it's a shield.
		break;
	};
	case(ItemType_Protector): {
		if (FindEquipType(dItemID, &pHelmetList[0])) dEquipSlot = 0x0C;
		else if (FindEquipType(dItemID, &pBodyList[0])) dEquipSlot = 0x0D;
		else if (FindEquipType(dItemID, &pHandsList[0])) dEquipSlot = 0x0E;
		else if (FindEquipType(dItemID, &pLegsList[0])) dEquipSlot = 0x0F;
		break;
	};
	case(ItemType_Accessory): {
		if (dRingSlotSelect >= 0x15) dRingSlotSelect = 0x11;
		dEquipSlot = dRingSlotSelect;
		dRingSlotSelect++;
		break;
	};
	case(ItemType_Goods): return false;
	default: {
		sprintf_s(pBuffer, "Invalid item type: %i (%08X)", dItemType, dItemID);
		Core->Panic(pBuffer, "...\\Source\\AutoEquip\\AutoEquip.cpp", HE_InvalidItemType, 0);
		return false;
	};
	
	};

	E->dEquipSlot = dEquipSlot;
	E->dInventorySlot = GetInventorySlotID(dItemID);

	if (E->dInventorySlot < 0) {
		sprintf_s(pBuffer, "Unable to find item: %08X", dItemID);
		Core->Panic(pBuffer, "...\\Source\\AutoEquip\\AutoEquip.cpp", HE_InvalidInventoryEquipID, 0);
		return false;
	};

	return true;
};

BOOL CAutoEquip::FindEquipType(DWORD dItem, DWORD* pArray) {

	int i = 0;

	if (!pArray) {
		Core->Panic("Null array pointer!", "...\\Source\\AutoEquip\\AutoEquip.cpp", FE_NullArray, 1);
		int3
	};

	while (*pArray) {
		if (dItem == *pArray) return true;
		pArray++;
	};

	return false;
};

DWORD CAutoEquip::GetInventorySlotID(DWORD dItemID) {

	DWORD dInventoryID = 0;
	UINT_PTR qInventoryPtr = 0;
	UINT_PTR qInventoryScanPtr = 0;

	qInventoryPtr = *(UINT_PTR*)CoreStruct->qLocalPlayer;
	qInventoryPtr = *(UINT_PTR*)(qInventoryPtr + 0x10);
	if (!qInventoryPtr) {
		Core->Panic("'Local Player' does not exist", "...\\Source\\AutoEquip\\AutoEquip.cpp", HE_NoPlayerChar, 1);
		int3
	};

	qInventoryPtr = *(UINT_PTR*)(qInventoryPtr + 0x470);
	qInventoryPtr = *(UINT_PTR*)(qInventoryPtr + 0x10);
	qInventoryPtr += 0x1B8;

	while (dInventoryID < *(DWORD*)(qInventoryPtr + 0x04)) {
	
		qInventoryScanPtr = (dInventoryID << 0x04);
		qInventoryScanPtr += *(UINT_PTR*)(qInventoryPtr + 0x38);

		if (*(DWORD*)(qInventoryScanPtr + 0x04) == dItemID) {
			return (dInventoryID + *(DWORD*)(qInventoryPtr + 0x14));
		};
	
		dInventoryID++;
	};

	return -1;
};

VOID CAutoEquip::LockUnlockEquipSlots(int iIsUnlock) {

	UINT_PTR qWorldChrMan = 0;
	DWORD dChrEquipAnimFlags = 0;

	qWorldChrMan = *(UINT_PTR*)(CoreStruct->qWorldChrMan);
	if (!qWorldChrMan) {
		Core->Panic("WorldChrMan", "...\\Source\\AutoEquip\\AutoEquip.cpp", HE_NoPlayerChar, 1);
		int3
	};

	qWorldChrMan = *(UINT_PTR*)(qWorldChrMan + 0x80);
	if (!qWorldChrMan) {
		Core->Panic("'WorldChr Player' does not exist", "...\\Source\\AutoEquip\\AutoEquip.cpp", HE_NoPlayerChar, 1);
		int3
	};

	qWorldChrMan = *(UINT_PTR*)(qWorldChrMan + 0x1F90);
	if (!qWorldChrMan) {
		Core->Panic("'WorldChr Data' does not exist", "...\\Source\\AutoEquip\\AutoEquip.cpp", HE_NoPlayerChar, 0);
		return;
	};

	qWorldChrMan = *(UINT_PTR*)(qWorldChrMan);
	if (!qWorldChrMan) {
		Core->Panic("'WorldChr Flags' does not exist", "...\\Source\\AutoEquip\\AutoEquip.cpp", HE_NoPlayerChar, 0);
		return;
	};

	dChrEquipAnimFlags = *(DWORD*)(qWorldChrMan + 0x10);

	if (iIsUnlock) dChrEquipAnimFlags |= 1;
	else dChrEquipAnimFlags &= 0xFFFFFFFE;

	*(DWORD*)(qWorldChrMan + 0x10) = dChrEquipAnimFlags;

	return;
};

extern DWORD pHelmetList[238]{

	0x1098BD90,
	0x1098E4A0,
	0x10990BB0,
	0x109932C0,
	0x109959D0,
	0x109980E0,
	0x1099A7F0,
	0x1099CF00,
	0x1099F610,
	0x109A1D20,
	0x109A4430,
	0x109A6B40,
	0x109A9250,
	0x109AB960,
	0x109AE070,
	0x109B0780,
	0x109B2E90,
	0x109B55A0,
	0x109B7CB0,
	0x109BA3C0,
	0x109BCAD0,
	0x109BF1E0,
	0x109C18F0,
	0x109C4000,
	0x109C6710,
	0x109CB530,
	0x109CDC40,
	0x109D0350,
	0x109D2A60,
	0x109D5170,
	0x109D7880,
	0x109D9F90,
	0x109DC6A0,
	0x109DEDB0,
	0x109E14C0,
	0x109E3BD0,
	0x109E89F0,
	0x1121EAC0,
	0x11298BE0,
	0x11312D00,
	0x11406F40,
	0x11481060,
	0x114FB180,
	0x115752A0,
	0x115EF3C0,
	0x115EFB90,
	0x116694E0,
	0x116E3600,
	0x1175D720,
	0x117D7840,
	0x11851960,
	0x118CBA80,
	0x11945BA0,
	0x119BFCC0,
	0x11A39DE0,
	0x11AB3F00,
	0x11B2E408,
	0x11BA8140,
	0x11C22260,
	0x11C9C380,
	0x11D905C0,
	0x11E84800,
	0x11F78A40,
	0x12625A00,
	0x12656740,
	0x129020C0,
	0x129F6300,
	0x12A70420,
	0x12AEA540,
	0x12BDE780,
	0x12CD29C0,
	0x12D4CAE0,
	0x12DC6C00,
	0x12E40D20,
	0x12EBAE40,
	0x130291A0,
	0x13197500,
	0x1328B740,
	0x1337F980,
	0x133F9AA0,
	0x13473BC0,
	0x134EDCE0,
	0x13567E00,
	0x135E1F20,
	0x1365C040,
	0x136D6160,
	0x13750280,
	0x137CA3A0,
	0x138BE5E0,
	0x13938700,
	0x139B2820,
	0x13A2C940,
	0x13AA6A60,
	0x13B20B80,
	0x13C14DC0,
	0x13C8EEE0,
	0x13D09000,
	0x13D83120,
	0x13EF1480,
	0x1405F7E0,
	0x140D9900,
	0x14153A20,
	0x142C1D80,
	0x143B5FC0,
	0x144AA200,
	0x1459E440,
	0x14692680,
	0x1470C7A0,
	0x147868C0,
	0x148009E0,
	0x1487AB00,
	0x148F4C20,
	0x1496ED40,
	0x149E8E60,
	0x14ADD0A0,
	0x14B571C0,
	0x14BD12E0,
	0x14C4B400,
	0x14CC5520,
	0x14D3F640,
	0x14DB9760,
	0x14E33880,
	0x14EAD9A0,
	0x14F27AC0,
	0x14FA1BE0,
	0x1501BD00,
	0x15095E20,
	0x1510FF40,
	0x15204180,
	0x152F83C0,
	0x153EC600,
	0x155D4A80,
	0x156C8CC0,
	0x157BCF00,
	0x158B1140,
	0x159A5380,
	0x15A995C0,
	0x15B8D800,
	0x15C81A40,
	0x15D75C80,
	0x15E69EC0,
	0x15E6C5D0,
	0x15E6ECE0,
	0x15E73B00,
	0x15E76210,
	0x15E78920,
	0x15EA4840,
	0x15E7D740,
	0x15E7FE50,
	0x15E82560,
	0x15E84C70,
	0x15E89A90,
	0x15E8C1A0,
	0x15E8E8B0,
	0x15E95DE0,
	0x15E984F0,
	0x15E9AC00,
	0x15E9D310,
	0x15E9FA20,
	0x15EA2130,
	0x15EA6F50,
	0x15EA9660,
	0x15EABD70,
	0x15EAE480,
	0x15EB0B90,
	0x15EB32A0,
	0x15EB59B0,
	0x15EB80C0,
	0x15EBA7D0,
	0x15EBCEE0,
	0x15EC1D00,
	0x15EC4410,
	0x15EC6B20,
	0x15EC9230,
	0x15ECB940,
	0x15ECE050,
	0x15ED0760,
	0x15ED2E70,
	0x15ED5580,
	0x15ED7C90,
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
	0x15F14D20,
	0x15F17430,
	0x15F19B40,
	0x15F1C250,
	0x15F1E960,
	0x15F21070,
	0x15F23780,
	0x15F25E90,
	0x15F285A0,
	0x15F2ACB0,
	0x15F2D3C0,
	0x15F2FAD0,
	0x15F321E0,
	0x15F348F0,
	0x15F37000,
	0x15F39710,
	0x15F3BE20,
	0x15F3E530,
	0x15F40C40,
	0x15F43350,
	0x109ED810,
	0x109EFF20,
	0x109F2630,
	0x109F4D40,
	0x109F7450,
	0x109F9B60,
	0x15EE3FE0,
	0x109FC270,
	0x10A01090,
	0x10A037A0,
	0x10A05EB0,
	0x10A085C0,
	0x10A0ACD0,
	0x00000000,

};		  //All in-game headwear

extern DWORD pBodyList[194]{

	0x1098C178,
	0x1098E888,
	0x10990F98,
	0x109936A8,
	0x10995DB8,
	0x109984C8,
	0x1099ABD8,
	0x1099D2E8,
	0x109A2108,
	0x109A4818,
	0x109A6F28,
	0x109A9638,
	0x109ABD48,
	0x109AE458,
	0x109B0B68,
	0x109C8E20,
	0x109D0738,
	0x109D2E48,
	0x109D5558,
	0x109D7C68,
	0x109DA378,
	0x109DCA88,
	0x109DF198,
	0x109E18A8,
	0x109E3FB8,
	0x109E66C8,
	0x109E8DD8,
	0x109EB4E8,
	0x1121EEA8,
	0x11298FC8,
	0x113130E8,
	0x11407328,
	0x11481448,
	0x114FB568,
	0x11575688,
	0x115EF7A8,
	0x115EFF78,
	0x116698C8,
	0x116E39E8,
	0x1175DB08,
	0x117D7C28,
	0x11851D48,
	0x118CBE68,
	0x11945F88,
	0x119C00A8,
	0x11A3A1C8,
	0x11AB42E8,
	0x11B2E7F0,
	0x11BA8528,
	0x11C22648,
	0x11C9C768,
	0x11CB4E08,
	0x11D909A8,
	0x11DA9048,
	0x11E84BE8,
	0x11F78E28,
	0x12625DE8,
	0x129F66E8,
	0x12A70808,
	0x12AEA928,
	0x12BDEB68,
	0x12CD2DA8,
	0x12DC6FE8,
	0x12E41108,
	0x12EBB228,
	0x13029588,
	0x131978E8,
	0x1328BB28,
	0x1337FD68,
	0x133F9E88,
	0x13473FA8,
	0x134EE0C8,
	0x135681E8,
	0x135E2308,
	0x1365C428,
	0x13750668,
	0x137CA788,
	0x138BE9C8,
	0x13938AE8,
	0x139B2C08,
	0x13A2CD28,
	0x13AA6E48,
	0x13B20F68,
	0x13C151A8,
	0x13C8F2C8,
	0x13D093E8,
	0x13D83508,
	0x13DFD628,
	0x13EF1868,
	0x1405FBC8,
	0x140D9CE8,
	0x142C2168,
	0x143B63A8,
	0x144AA5E8,
	0x1459E828,
	0x14692A68,
	0x1470CB88,
	0x14786CA8,
	0x14800DC8,
	0x1487AEE8,
	0x148F5008,
	0x1496F128,
	0x149E9248,
	0x14A63368,
	0x14B575A8,
	0x14BD16C8,
	0x14C4B7E8,
	0x14CC5908,
	0x14D3FA28,
	0x14DB9B48,
	0x14E33C68,
	0x14EADD88,
	0x14F27EA8,
	0x1501C0E8,
	0x15110328,
	0x15204568,
	0x152F87A8,
	0x153EC9E8,
	0x154E0C28,
	0x155D4E68,
	0x156C90A8,
	0x157BD2E8,
	0x158B1528,
	0x159A5768,
	0x15A999A8,
	0x15B8DBE8,
	0x15C81E28,
	0x15D76068,
	0x15E6A2A8,
	0x15E6C9B8,
	0x15E6F0C8,
	0x15E80238,
	0x15E87380,
	0x15E961C8,
	0x15E988D8,
	0x15E9AFE8,
	0x15E9D6F8,
	0x15E9FE08,
	0x15EA2518,
	0x15EA4C28,
	0x15EA7338,
	0x15EA9A48,
	0x15EAC158,
	0x15EAE868,
	0x15EB0F78,
	0x15EB3688,
	0x15EB5D98,
	0x15EB84A8,
	
	0x15EC20E8,
	0x15EC9618,
	0x15ECBD28,
	0x15ECE438,
	0x15ED0B48,
	0x15ED3258,
	0x15ED5968,
	0x15ED8078,
	0x15F129F8,
	0x15F15108,
	0x15F17818,
	0x15F19F28,
	0x15F1C638,
	0x15F1ED48,
	0x15F21458,
	0x15F23B68,
	0x15F26278,
	0x15F28988,
	0x15F2B098,
	0x15F2D7A8,
	0x15F2FEB8,
	0x15F325C8,
	0x15F34CD8,
	0x15F373E8,
	0x15F39AF8,
	0x15F3C208,
	0x15F41028,
	0x15F43738,
	0x109EDBF8,
	0x109F0308,
	0x109F2A18,
	0x109F5128,
	0x109F7838,
	0x109F9F48,
	0x109FED68,
	0x15EE43C8,
	0x109FC658,
	0x109FED68,
	0x10A01478,
	0x10A037A0,
	0x10A03B88,
	0x10A06298,
	0x10A089A8,
	0x10A0B0B8,
	0x13F6B988,
	0x00000000,

};		  //All in-game chestpieces

extern DWORD pHandsList[181]{

	0x1098C560,
	0x1098EC70,
	0x10991380,
	0x10993A90,
	0x109961A0,
	0x109988B0,
	0x1099AFC0,
	0x1099D6D0,
	0x109A24F0,
	0x109A4C00,
	0x109A7310,
	0x109A9A20,
	0x109AC130,
	0x109AE840,
	0x109D0B20,
	0x109D3230,
	0x109D5940,
	0x109D8050,
	0x109DA760,
	0x109DCE70,
	0x109DF580,
	0x109E1C90,
	0x109E43A0,
	0x109E6AB0,
	0x109E91C0,
	0x109EB8D0,
	0x1121F290,
	0x112993B0,
	0x113134D0,
	0x11407710,
	0x11481830,
	0x114FB950,
	0x11575A70,
	0x11607A60,
	0x11669CB0,
	0x116E3DD0,
	0x1175DEF0,
	0x117D8010,
	0x11852130,
	0x118CC250,
	0x11946370,
	0x119C0490,
	0x11A3A5B0,
	0x11AB46D0,
	0x11BA8910,
	0x11C22A30,
	0x11C9CB50,
	0x11D90D90,
	0x11E84FD0,
	0x11F79210,
	0x129F6AD0,
	0x12AEAD10,
	0x12BDEF50,
	0x12CD3190,
	0x12DC73D0,
	0x12EBB610,
	0x13029970,
	0x13197CD0,
	0x1328BF10,
	0x13380150,
	0x133FA270,
	0x13474390,
	0x134EE4B0,
	0x135685D0,
	0x1365C810,
	0x13750A50,
	0x137CAB70,
	0x13938ED0,
	0x139B2FF0,
	0x13A2D110,
	0x13B21350,
	0x13C15590,
	0x13C8F6B0,
	0x13D097D0,
	0x13D838F0,
	0x13EF2038,
	0x1405FFB0,
	0x140DA0D0,
	0x142C2550,
	0x143B6790,
	0x144AA9D0,
	0x1459EC10,
	0x14692E50,
	0x1470CF70,
	0x14787090,
	0x148011B0,
	0x1487B2D0,
	0x148F53F0,
	0x1496F510,
	0x149E9630,
	0x14A63750,
	0x14B57990,
	0x14BD1AB0,
	0x14C4BBD0,
	0x14CC5CF0,
	0x14D3FE10,
	0x14DB9F30,
	0x14E34050,
	0x14EAE170,
	0x14F28290,
	0x1501C4D0,
	0x15110710,
	0x15204950,
	0x152F8B90,
	0x153ECDD0,
	0x154E1010,
	0x155D5250,
	0x156C9490,
	0x157BD6D0,
	0x158B1910,
	0x159A5B50,
	0x15A99D90,
	0x15B8DFD0,
	0x15C82210,
	0x15D76450,
	0x15E6A690,
	0x15E6CDA0,
	0x15E6F4B0,
	0x15E713F0,
	0x15E80620,
	0x15E965B0,
	0x15E98CC0,
	0x15E9B3D0,
	0x15E9DAE0,
	0x15EA01F0,
	0x15EA2900,
	0x15EA5010,
	0x15EA7720,
	0x15EA9E30,
	0x15EAC540,
	0x15EAEC50,
	0x15EB1360,
	0x15EB3A70,
	0x15EB6180,
	0x15EB8890,
	0x15EBF5F0,
	0x15EC24D0,
	0x15EC9A00,
	0x15ECC110,
	0x15ECE820,
	0x15ED0F30,
	0x15ED3640,
	0x15ED5D50,
	0x15ED8460,
	0x15F12DE0,
	0x15F154F0,
	0x15F17C00,
	0x15F1A310,
	0x15F1CA20,
	0x15F1F130,
	0x15F21840,
	0x15F23F50,
	0x15F26660,
	0x15F28D70,
	0x15F2B480,
	0x15F2DB90,
	0x15F302A0,
	0x15F329B0,
	0x15F350C0,
	0x15F377D0,
	0x15F39EE0,
	0x15F3C5F0,
	0x15F41410,
	0x15F43B20,
		0x109EDFE0,
		0x109F06F0,
		0x109F2E00,
		0x109F5510,
		0x109F7C20,
		0x109FA330,
		0x109FF150,
		0x15EE47B0,
		0x109FF150,
		0x10A01860,
		0x10A03F70,
		0x10A06680,
		0x10A08D90,
		0x10A09178,
		0x10A0B4A0,
		0x15EBD6B0,
	0x00000000,
};

extern DWORD pLegsList[182]{

	0x1098C948,
	0x1098F058,
	0x10991768,
	0x10993E78,
	0x10996588,
	0x10998C98,
	0x1099B3A8,
	0x1099DAB8,
	0x109A28D8,
	0x109A4FE8,
	0x109A76F8,
	0x109A9E08,
	0x109AC518,
	0x109AEC28,
	0x109D0F08,
	0x109D3618,
	0x109D5D28,
	0x109D8438,
	0x109DAB48,
	0x109DD258,
	0x109DF968,
	0x109E2078,
	0x109E4788,
	0x109E6E98,
	0x109E95A8,
	0x109EBCB8,
	0x1121F678,
	0x11299798,
	0x113138B8,
	0x11407AF8,
	0x11481C18,
	0x114FBD38,
	0x11575E58,
	0x11607E48,
	0x1166A098,
	0x116E41B8,
	0x1175E2D8,
	0x117D83F8,
	0x11852518,
	0x118CC638,
	0x11946758,
	0x119C0878,
	0x11A3A998,
	0x11AB4AB8,
	0x11B2EBD8,
	0x11BA8CF8,
	0x11C22E18,
	0x11C9CF38,
	0x11D91178,
	0x11E853B8,
	0x11F795F8,
	0x126265B8,
	0x129F6EB8,
	0x12AEB0F8,
	0x12BDF338,
	0x12CD3578,
	0x12DC77B8,
	0x12EBB9F8,
	0x13029D58,
	0x131980B8,
	0x1328C2F8,
	0x13380538,
	0x133FA658,
	0x13474778,
	0x134EE898,
	0x135689B8,
	0x135E2AD8,
	0x1365CBF8,
	0x13750E38,
	0x137CAF58,
	0x139392B8,
	0x139B33D8,
	0x13A2D4F8,
	0x13AA7618,
	0x13B21738,
	0x13C15978,
	0x13C8FA98,
	0x13D09BB8,
	0x13D83CD8,
	0x13F6C158,
	0x14060398,
	0x140DA4B8,
	0x142C2938,
	0x143B6B78,
	0x144AADB8,
	0x1459EFF8,
	0x14693238,
	0x1470D358,
	0x14787478,
	0x14801598,
	0x1487B6B8,
	0x148F57D8,
	0x1496F8F8,
	0x149E9A18,
	0x14A63B38,
	0x14B57D78,
	0x14BD1E98,
	0x14C4BFB8,
	0x14CC60D8,
	0x14D401F8,
	0x14DBA318,
	0x14E34438,
	0x14EAE558,
	0x14F28678,
	0x1501C8B8,
	0x15110AF8,
	0x15204D38,
	0x152F8F78,
	0x153ED1B8,
	0x154E13F8,
	0x155D5638,
	0x156C9878,
	0x157BDAB8,
	0x158B1CF8,
	0x159A5F38,
	0x15A9A178,
	0x15B8E3B8,
	0x15C825F8,
	0x15D76838,
	0x15E6AA78,
	0x15E6D188,
	0x15E6F898,
	0x15E80A08,
	0x15E96998,
	0x15E990A8,
	0x15E9B7B8,
	0x15E9DEC8,
	0x15EA05D8,
	0x15EA2CE8,
	0x15EA53F8,
	0x15EA7B08,
	0x15EAA218,
	0x15EAC928,
	0x15EAF038,
	0x15EB1748,
	0x15EB3E58,
	0x15EB6568,
	0x15EB8C78,
	0x15EC28B8,
	0x15EC9DE8,
	0x15ECC4F8,
	0x15ECEC08,
	0x15ED1318,
	0x15ED3A28,
	0x15ED6138,
	0x15ED8848,
	0x15F131C8,
	0x15F158D8,
	0x15F17FE8,
	0x15F1A6F8,
	0x15F1CE08,
	0x15F1F518,
	0x15F21C28,
	0x15F24338,
	0x15F26A48,
	0x15F29158,
	0x15F2B868,
	0x15F2DF78,
	0x15F30688,
	0x15F32D98,
	0x15F354A8,
	0x15F37BB8,
	0x15F3A2C8,
	0x15F3C9D8,
	0x15F417F8,
	0x15F43F08,
	0x109EE3C8,
	0x109F0AD8,
	0x109F31E8,
	0x109F58F8,
	0x109F8008,
	0x109FA718,
	0x109FF538,
	0x15EE4B98,
	0x109FCE28,
		0x109FF538,
		0x10A01C48,
		0x10A04358,
		0x10A06A68,
		0x10A09178,
		0x10A0B888,
	0x00000000,
};