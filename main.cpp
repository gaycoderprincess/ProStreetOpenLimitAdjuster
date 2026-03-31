#include <windows.h>
#include <format>
#include <filesystem>
#include <toml++/toml.hpp>

#include "nya_commonhooklib.h"
#include "nfsps.h"

#include "chloemenulib.h"

void WriteLog(const std::string& str) {
	static auto file = std::ofstream("NFSPSOpenLimitAdjuster_gcp.log");

	file << str;
	file << "\n";
	file.flush();
}

void __thiscall NewVector_Destruct(UTL::Vector<void*>* pThis, int a2) {
	if ((a2 & 1) == 0) return;

	if (!pThis->mBegin) return;
	free(pThis->mBegin);
}

template<size_t count>
void* __thiscall NewVector_AllocVectorSpace(UTL::Vector<void*>* pThis, size_t num, size_t alignment) {
	//WriteLog(std::format("AllocVectorSpace {:X}", (uintptr_t)__builtin_return_address(0)));
	return malloc(num * count);
}

void __thiscall NewVector_FreeVectorSpace(UTL::Vector<void*>* pThis, void* buffer, size_t num) {
	return free(buffer);
}

size_t __thiscall NewVector_GetGrowSize(UTL::Vector<void*>* pThis, size_t minSize) {
	return std::max(minSize, pThis->mCapacity);
}

size_t __thiscall NewVector_GetMaxCapacity(UTL::Vector<void*>* pThis) {
	return 0x7FFFFFFF;
}

void __thiscall NewVector_OnGrowRequest(UTL::Vector<void*>* pThis, size_t newSize) {
	return;
}

void* aNewVectorVTable[] = {
	(void*)&NewVector_Destruct,
	(void*)&NewVector_AllocVectorSpace<4>,
	(void*)&NewVector_FreeVectorSpace,
	(void*)&NewVector_GetGrowSize,
	(void*)&NewVector_GetMaxCapacity,
	(void*)&NewVector_OnGrowRequest,
};

const char* aSlotPoolNames[] = {
		"AnimCtrlSlotPool",
		"Anim_CAnimSkeleton_SlotPool",
		"Anim_CNFSAnimBank_SlotPool",
		"AStarSearchRequestSlotPool",
		"AUD:Csis SlotPools",
		"bFile System",
		"CarEmitterPositionSlotPool",
		"CarLoadedRideInfoSlotPool",
		"CarLoadedSkinLayerSlotPool",
		"CarLoadedSolidPackSlotPool",
		"CarLoadedTexturePackSlotPool",
		"CarPartModelPool",
		"ClanSlotPool",
		"CustomizationLightMaterialPool",
		"DynamicSurface",
		"eAnimTextureSlotPool",
		"eMeshRender",
		"EventHandlerSlotPool",
		"EventSlotPool",
		"FERenderObjectSlotPool",
		"FERenderPolySlotPool",
		"LoadedVinylLayersSlotPool",
		"QueuedFileSlotPool",
		"ResourceFileSlotPool",
		"ScheduledSpeechEvent slotpool",
		"SkidSetSlotPool",
		"SpaceNodeSlotPool",
		"SpeechSampleData slotpool",
		"VehicleDamagePartSlotPool",
		"VehiclePartDamageZoneSlotPool",
		"VoiceActors slotpool",
		"WorldAnimCtrl_SlootPol",
		"WorldModelSlotPool",
};
size_t aSlotPoolSizes[sizeof(aSlotPoolNames)/sizeof(aSlotPoolNames[0])] = {};

SlotPool* bNewSlotPoolHooked(int slot_size, int num_slots, const char *debug_name, int memory_pool) {
	if (debug_name) {
		//WriteLog(std::format("{}={}", debug_name, num_slots));
		for (int i = 0; i < sizeof(aSlotPoolNames)/sizeof(aSlotPoolNames[0]); i++) {
			if (!aSlotPoolSizes[i]) continue;
			if (strcmp(debug_name, aSlotPoolNames[i])) continue;

			num_slots = aSlotPoolSizes[i];
		}
	}
	return bNewSlotPool(slot_size, num_slots, debug_name, memory_pool);
}

int nMaxVehicles = 20;
uintptr_t PVehicleMakeRoomASM_jmp = 0x728780;
void __attribute__((naked)) __fastcall PVehicleMakeRoomASM() {
	__asm__ (
		"mov eax, [esp+0x1C]\n\t"
		"add eax, ecx\n\t"
		"push ebx\n\t"
		"mov ebx, dword ptr %1\n\t"
		"cmp eax, ebx\n\t"
		"jbe loc_687823\n\t"
		"mov edx, eax\n\t"
		"sub edx, ebx\n\t"
		"mov [esp+0x18], edx\n\t"

	"loc_687823:\n\t"
		"pop ebx\n\t"
		"jmp %0\n\t"
			:
			: "m" (PVehicleMakeRoomASM_jmp), "m" (nMaxVehicles)
	);
}

void DebugMenu() {
	ChloeMenuLib::BeginMenu();

	DrawMenuOption(std::format("VEHICLE_ALL: {}", VEHICLE_LIST::GetList(VEHICLE_ALL).size()));
	DrawMenuOption(std::format("VEHICLE_PLAYERS: {}", VEHICLE_LIST::GetList(VEHICLE_PLAYERS).size()));
	DrawMenuOption(std::format("VEHICLE_AI: {}", VEHICLE_LIST::GetList(VEHICLE_AI).size()));
	DrawMenuOption(std::format("VEHICLE_AIRACERS: {}", VEHICLE_LIST::GetList(VEHICLE_AIRACERS).size()));
	DrawMenuOption(std::format("VEHICLE_AICOPS: {}", VEHICLE_LIST::GetList(VEHICLE_AICOPS).size()));
	DrawMenuOption(std::format("VEHICLE_AITRAFFIC: {}", VEHICLE_LIST::GetList(VEHICLE_AITRAFFIC).size()));
	DrawMenuOption(std::format("VEHICLE_RACERS: {}", VEHICLE_LIST::GetList(VEHICLE_RACERS).size()));
	DrawMenuOption(std::format("VEHICLE_REMOTE: {}", VEHICLE_LIST::GetList(VEHICLE_REMOTE).size()));
	DrawMenuOption(std::format("VEHICLE_INACTIVE: {}", VEHICLE_LIST::GetList(VEHICLE_INACTIVE).size()));
	DrawMenuOption(std::format("VEHICLE_TRAILERS: {}", VEHICLE_LIST::GetList(VEHICLE_TRAILERS).size()));
	//DrawMenuOption(std::format("RigidBody: {}", RigidBody::mCount));
	//DrawMenuOption(std::format("SimpleRigidBody: {}", SimpleRigidBody::mCount));
	DrawMenuOption(std::format("SimTask: {}", *(int*)0xFFD724));
	//DrawMenuOption(std::format("Free Memory: {}K", bCountFreeMemory(0) >> 10));

	ChloeMenuLib::EndMenu();
}

uint32_t FindVectorVTableFromLocator(uint32_t addr) {
	auto p = (uint8_t*)&addr;
	std::vector<uint16_t> pattern;
	pattern.push_back(p[0]);
	pattern.push_back(p[1]);
	pattern.push_back(p[2]);
	pattern.push_back(p[3]);
	pattern.push_back(0x1FF);
	pattern.push_back(0x1FF);
	pattern.push_back(0x1FF);
	pattern.push_back(0x1FF);
	pattern.push_back(0xC0);
	pattern.push_back(0x10);
	pattern.push_back(0x6C);
	pattern.push_back(0x00);

	auto realAddr = NyaHookLib::SigScanner::FindSignature(pattern);
	if (!realAddr) {
		pattern.pop_back();
		pattern.pop_back();
		pattern.pop_back();
		pattern.pop_back();
		pattern.push_back(0xE0);
		pattern.push_back(0xF5);
		pattern.push_back(0x55);
		pattern.push_back(0x00);
		realAddr = NyaHookLib::SigScanner::FindSignature(pattern);
	}

	if (!realAddr) {
		MessageBoxA(nullptr, std::format("Failed to find pattern for {:X}", addr).c_str(), "nya?!~", MB_ICONERROR);
		return 0;
	}
	return realAddr;
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x16AA080 && NyaHookLib::GetEntryPoint() != 0x428C25) {
				MessageBoxA(nullptr, "Unsupported game version! Make sure you're using v1.1 (.exe size of 3765248 or 28739656 bytes)", "nya?!~", MB_ICONERROR);
				return TRUE;
			}

			ChloeMenuLib::RegisterMenu("Open Limit Adjuster Debug", &DebugMenu);

			if (std::filesystem::exists("NFSPSOpenLimitAdjuster_gcp.toml")) {
				try {
					toml::parse_file("NFSPSOpenLimitAdjuster_gcp.toml");
				}
				catch (const toml::parse_error& err) {
					MessageBoxA(0, std::format("Failed to parse config: {}", err.what()).c_str(), "nya?!~", MB_ICONERROR);
				}

				auto config = toml::parse_file("NFSPSOpenLimitAdjuster_gcp.toml");
				CarLoaderPoolSizes = config["car_loader_memory"].value_or(CarLoaderPoolSizes);

				for (int i = 0; i < sizeof(aSlotPoolNames)/sizeof(aSlotPoolNames[0]); i++) {
					std::string str = aSlotPoolNames[i];
					for (auto& c : str) {
						if (c == ':' || c == ' ') c = '_';
					}

					aSlotPoolSizes[i] = config["slot_pools"][str].value_or(0);
				}

				nMaxVehicles = config["vehicle_count"].value_or(20);

				int numDispatcher = config["collision_dispatcher"].value_or(256);
				NyaHookLib::Patch<uint32_t>(0x4CF0D6 + 1, (numDispatcher * 0x10) + 0x10); // malloc
				NyaHookLib::Patch<uint32_t>(0x4CF107 + 1, (numDispatcher * 0x10) + 0x10); // memset
				NyaHookLib::Patch<uint32_t>(0x4CF125 + 1, (numDispatcher * 0x10) + 0x10); // vector capacity

				NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x72876E, &PVehicleMakeRoomASM);
			}

			// remove limit from SimTask
			NyaHookLib::Patch<uint8_t>(0x4CE760, 0xEB);

			uintptr_t vtables[] = {
				0xA11388, // UTL::FixedVector<ActionQueue *,32,16>::`RTTI Complete Object Locator'
				0xA0AC28, // UTL::FixedVector<CameraAI::Director *,2,16>::`RTTI Complete Object Locator'
				0x9FCC80, // UTL::FixedVector<CarSoundConn *,12,16>::`RTTI Complete Object Locator'
				0xA1518C, // UTL::FixedVector<Csis::Type_RepPointValue,4,16>::`RTTI Complete Object Locator'
				0x9FCC34, // UTL::FixedVector<EAX_CarState *,12,16>::`RTTI Complete Object Locator'
				0x9FE0E4, // UTL::FixedVector<EngineMappingPair,24,16>::`RTTI Complete Object Locator'
				0xA162D8, // UTL::FixedVector<ExtrapolatedCar *,12,16>::`RTTI Complete Object Locator'
				0xA0C5A0, // UTL::FixedVector<GProgressionActivityWrapper *,100,16>::`RTTI Complete Object Locator'
				0xA131BC, // UTL::FixedVector<ICollisionBody *,256,16>::`RTTI Complete Object Locator'
				0xA0AC74, // UTL::FixedVector<IDebugWatchCar *,2,16>::`RTTI Complete Object Locator'
				0xA132EC, // UTL::FixedVector<IDisposable *,256,16>::`RTTI Complete Object Locator'
				0xA13598, // UTL::FixedVector<IDrafter *,12,16>::`RTTI Complete Object Locator'
				0xA13124, // UTL::FixedVector<IExplosion *,128,16>::`RTTI Complete Object Locator'
				0xA0C684, // UTL::FixedVector<IGameplayStats *,5,16>::`RTTI Complete Object Locator'
				0xA1341C, // UTL::FixedVector<IHud *,2,16>::`RTTI Complete Object Locator'
				0xA13468, // UTL::FixedVector<IInputPlayer *,12,16>::`RTTI Complete Object Locator'
				0xA11A68, // UTL::FixedVector<ILuaBindable *,20,16>::`RTTI Complete Object Locator'
				0xA130D8, // UTL::FixedVector<IModel *,2296,16>::`RTTI Complete Object Locator'
				0xA14EE0, // UTL::FixedVector<int,7,16>::`RTTI Complete Object Locator'
				0xA12FA8, // UTL::FixedVector<IPlayer *,12,16>::`RTTI Complete Object Locator'
				0xA13338, // UTL::FixedVector<IRecordablePlayer *,12,16>::`RTTI Complete Object Locator'
				0xA13254, // UTL::FixedVector<IRigidBody *,256,16>::`RTTI Complete Object Locator'
				0xA0C5EC, // UTL::FixedVector<ISelectionSet *,5,16>::`RTTI Complete Object Locator'
				0xA1DD0C, // UTL::FixedVector<ISimable *,12,16>::`RTTI Complete Object Locator'
				0xA13208, // UTL::FixedVector<ISimpleBody *,128,16>::`RTTI Complete Object Locator'
				0x9FE214, // UTL::FixedVector<ISndAttachable *,128,16>::`RTTI Complete Object Locator'
				0xA13384, // UTL::FixedVector<ISpikeable *,12,16>::`RTTI Complete Object Locator'
				0xA05A58, // UTL::FixedVector<IUnplugErrorAllowable *,20,16>::`RTTI Complete Object Locator'
				0xA13170, // UTL::FixedVector<IVehicle *,12,16>::`RTTI Complete Object Locator'
				0xA134B4, // UTL::FixedVector<IVehicleCache *,20,16>::`RTTI Complete Object Locator'
				0xA133D0, // UTL::FixedVector<IWheelDamage *,12,16>::`RTTI Complete Object Locator'
				0xA0C638, // UTL::FixedVector<ScriptDebuggable *,10,16>::`RTTI Complete Object Locator'
				0xA12FF4, // UTL::FixedVector<Sim::IEntity *,12,16>::`RTTI Complete Object Locator'
				0xA1354C, // UTL::FixedVector<Smackable *,256,16>::`RTTI Complete Object Locator'
				0x9FCCCC, // UTL::FixedVector<Sound::CollisionEvent *,32,16>::`RTTI Complete Object Locator'
				0xA1505C, // UTL::FixedVector<SPCHType_1_EventID,10,16>::`RTTI Complete Object Locator'
				0xA15010, // UTL::FixedVector<SPCHType_1_EventID,8,16>::`RTTI Complete Object Locator'
				0xA15140, // UTL::FixedVector<uint,23,16>::`RTTI Complete Object Locator'
				0x9FE130, // UTL::FixedVector<uint,8,16>::`RTTI Complete Object Locator'
				0xA16324, // UTL::FixedVector<VehicleRenderConn *,12,16>::`RTTI Complete Object Locator'
				0xA16EA0, // UTL::FixedVector<WCollider *,256,16>::`RTTI Complete Object Locator'
				0xA116A0, // UTL::Listable<ActionQueue,32>::List::`RTTI Complete Object Locator'
				0xA0B270, // UTL::Listable<CameraAI::Director,2>::List::`RTTI Complete Object Locator'
				0x9FCFAC, // UTL::Listable<CarSoundConn,12>::List::`RTTI Complete Object Locator'
				0x9FCF5C, // UTL::Listable<EAX_CarState,12>::List::`RTTI Complete Object Locator'
				0xA0D16C, // UTL::Listable<GProgressionActivityWrapper,100>::List::`RTTI Complete Object Locator'
				0xA13BD8, // UTL::Listable<ICollisionBody,256>::List::`RTTI Complete Object Locator'
				0xA0B2C0, // UTL::Listable<IDebugWatchCar,2>::List::`RTTI Complete Object Locator'
				0xA13D18, // UTL::Listable<IDisposable,256>::List::`RTTI Complete Object Locator'
				0xA13B88, // UTL::Listable<IExplosion,128>::List::`RTTI Complete Object Locator'
				0xA0D25C, // UTL::Listable<IGameplayStats,5>::List::`RTTI Complete Object Locator'
				0xA13E58, // UTL::Listable<IHud,2>::List::`RTTI Complete Object Locator'
				0xA13EA8, // UTL::Listable<IInputPlayer,12>::List::`RTTI Complete Object Locator'
				0xA11CA4, // UTL::Listable<ILuaBindable,20>::List::`RTTI Complete Object Locator'
				0xA13B38, // UTL::Listable<IModel,2296>::List::`RTTI Complete Object Locator'
				0xA13D68, // UTL::Listable<IRecordablePlayer,12>::List::`RTTI Complete Object Locator'
				0xA13C78, // UTL::Listable<IRigidBody,256>::List::`RTTI Complete Object Locator'
				0xA0D1BC, // UTL::Listable<ISelectionSet,5>::List::`RTTI Complete Object Locator'
				0xA13C28, // UTL::Listable<ISimpleBody,128>::List::`RTTI Complete Object Locator'
				0x9FEB04, // UTL::Listable<ISndAttachable,128>::List::`RTTI Complete Object Locator'
				0xA13DB8, // UTL::Listable<ISpikeable,12>::List::`RTTI Complete Object Locator'
				0xA070A0, // UTL::Listable<IUnplugErrorAllowable,20>::List::`RTTI Complete Object Locator'
				0xA13EF8, // UTL::Listable<IVehicleCache,20>::List::`RTTI Complete Object Locator'
				0xA13E08, // UTL::Listable<IWheelDamage,12>::List::`RTTI Complete Object Locator'
				0xA0D20C, // UTL::Listable<ScriptDebuggable,10>::List::`RTTI Complete Object Locator'
				0xA13F48, // UTL::Listable<Smackable,256>::List::`RTTI Complete Object Locator'
				0x9FCFFC, // UTL::Listable<Sound::CollisionEvent,32>::List::`RTTI Complete Object Locator'
				0xA1650C, // UTL::Listable<VehicleRenderConn,12>::List::`RTTI Complete Object Locator'
				0xA170EC, // UTL::Listable<WCollider,256>::List::`RTTI Complete Object Locator'
				0xA16414, // UTL::ListableSet<ExtrapolatedCar,12,eExtrapolatedCarClass,1>::List::`RTTI Complete Object Locator'
				0xA136D4, // UTL::ListableSet<IDrafter,12,eDraftableList,16>::List::`RTTI Complete Object Locator'
				0xA135E4, // UTL::ListableSet<IPlayer,12,ePlayerList,3>::List::`RTTI Complete Object Locator'
				0xA13684, // UTL::ListableSet<IVehicle,12,eVehicleList,12>::List::`RTTI Complete Object Locator'
				0xA13634, // UTL::ListableSet<Sim::IEntity,12,eEntityList,4>::List::`RTTI Complete Object Locator'
			};
			for (auto& addr : vtables) {
				auto realAddr = FindVectorVTableFromLocator(addr);
				if (!realAddr) continue;

				for (int i = 0; i < 6; i++) {
					NyaHookLib::Patch(realAddr + 4 + (i * 4), aNewVectorVTable[i]);
				}
			}

			uintptr_t garbagevtables[] = {
					0x9FE1C8, // UTL::FixedVector<CarStateObjPair,12,16>::`RTTI Complete Object Locator'
					0x9FE17C, // UTL::FixedVector<CSTATEMGR_CarState::EngToCarStruct,24,16>::`RTTI Complete Object Locator'
					0xA150A8, // UTL::FixedVector<Speech::SpeechEventPair,569,16>::`RTTI Complete Object Locator'
					0xA14F2C, // UTL::FixedVector<Speech::SpeechEventParmPair,69,16>::`RTTI Complete Object Locator'
					0xA13500, // UTL::FixedVector<UTL::GarbageNode<PhysicsObject,256>::Collector::_Node,256,16>::`RTTI Complete Object Locator'
					0xA27EA8, // UTL::FixedVector<UTL::GarbageNode<Sim::Activity,40>::Collector::_Node,40,16>::`RTTI Complete Object Locator'
					0xA27B34, // UTL::FixedVector<UTL::GarbageNode<Sim::Entity,12>::Collector::_Node,12,16>::`RTTI Complete Object Locator'
					0xA27A3C, // UTL::FixedVector<UTL::GarbageNode<Sim::Model,2296>::Collector::_Node,2296,16>::`RTTI Complete Object Locator'
					0xA132A0, // UTL::FixedVector<UTL::_KeyedNode,12,16>::`RTTI Complete Object Locator'
					0xA1308C, // UTL::FixedVector<UTL::_KeyedNode,2296,16>::`RTTI Complete Object Locator'
					0xA12F5C, // UTL::FixedVector<UTL::_KeyedNode,256,16>::`RTTI Complete Object Locator'
					0xA113D4, // UTL::FixedVector<UTL::_KeyedNode,32,16>::`RTTI Complete Object Locator'
					0xA13040, // UTL::FixedVector<UTL::_KeyedNode,40,16>::`RTTI Complete Object Locator'
					0xA116F0, // UTL::Instanceable<HACTIONQUEUE__ *,ActionQueue,32>::_List::`RTTI Complete Object Locator'
					0xA13A98, // UTL::Instanceable<HACTIVITY__ *,Sim::IActivity,40>::_List::`RTTI Complete Object Locator'
					0xA13CC8, // UTL::Instanceable<HCAUSE__ *,ICause,12>::_List::`RTTI Complete Object Locator'
					0xA13AE8, // UTL::Instanceable<HMODEL__ *,IModel,2296>::_List::`RTTI Complete Object Locator'
					0xA13A48, // UTL::Instanceable<HSIMABLE__ *,ISimable,256>::_List::`RTTI Complete Object Locator'
			};
			for (auto& addr : garbagevtables) {
				auto realAddr = FindVectorVTableFromLocator(addr);
				if (!realAddr) continue;

				for (int i = 0; i < 6; i++) {
					auto func = aNewVectorVTable[i];
					if (i == 1) func = (void*)&NewVector_AllocVectorSpace<8>;
					NyaHookLib::Patch(realAddr + 4 + (i * 4), func);
				}
			}

			uintptr_t vtables_3[] = {
					0xA00EBC, // UTL::FixedVector<JukeboxEntry,40,16>::`RTTI Complete Object Locator'
					0xA150F4, // UTL::FixedVector<Speech::HistoryPair,569,16>::`RTTI Complete Object Locator'
					0xA14FC4, // UTL::FixedVector<Speech::SpeechHubOpponentInfo,23,16>::`RTTI Complete Object Locator'
			};
			for (auto& addr : vtables_3) {
				auto realAddr = FindVectorVTableFromLocator(addr);
				if (!realAddr) continue;

				for (int i = 0; i < 6; i++) {
					auto func = aNewVectorVTable[i];
					if (i == 1) func = (void*)&NewVector_AllocVectorSpace<0xC>;
					NyaHookLib::Patch(realAddr + 4 + (i * 4), func);
				}
			}

			uintptr_t vtables_4[] = {
					0xA14F78, // UTL::FixedVector<Speech::SpeechRaceOpponentInfo,10,16>::`RTTI Complete Object Locator'
			};
			for (auto& addr : vtables_4) {
				auto realAddr = FindVectorVTableFromLocator(addr);
				if (!realAddr) continue;

				for (int i = 0; i < 6; i++) {
					auto func = aNewVectorVTable[i];
					if (i == 1) func = (void*)&NewVector_AllocVectorSpace<0x10>;
					NyaHookLib::Patch(realAddr + 4 + (i * 4), func);
				}
			}

			uintptr_t vtables_6[] = {
					0xA12F10, // UTL::FixedVector<PVehicle::ManageNode,12,16>::`RTTI Complete Object Locator'
					0xA139F8, // PVehicle::ManagementList::`RTTI Complete Object Locator'
			};
			for (auto& addr : vtables_6) {
				auto realAddr = FindVectorVTableFromLocator(addr);
				if (!realAddr) continue;

				for (int i = 0; i < 6; i++) {
					auto func = aNewVectorVTable[i];
					if (i == 1) func = (void*)&NewVector_AllocVectorSpace<0x18>;
					NyaHookLib::Patch(realAddr + 4 + (i * 4), func);
				}
			}

			uintptr_t slotpools[] = {
					0x4E08A7,
					0x4E0FA9,
					0x4E3ED9,
					0x4FD690,
					0x5900BD,
					0x5A0A37,
					0x5A0A52,
					0x6CBDAB,
					0x6CCDBE,
					0x6CCFA1,
					0x6CD3D4,
					0x6FBB2E,
					0x6FBB4D,
					0x73D2D6,
					0x73D301,
					0x742B99,
					0x74339C,
					0x743444,
					0x7457BB,
					0x7457D0,
					0x7510CE,
					0x75129F,
					0x754BE1,
					0x755CCB,
					0x755CE0,
					0x755CF8,
					0x755D10,
					0x755D2B,
					0x7579FA,
					0x75FA4E,
					0x75FA63,
					0x75FECA,
					0x7A098D,
					0x7AD79B,
			};
			for (auto& addr : slotpools) {
				NyaHookLib::PatchRelative(NyaHookLib::CALL, addr, &bNewSlotPoolHooked);
			}
			NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x1E38016, &bNewSlotPoolHooked);

			WriteLog("Mod initialized");
		} break;
		default:
			break;
	}
	return TRUE;
}