#include "hooks.h"

// include minhook for epic hookage
#include "../../ext/minhook/minhook.h"

#include <intrin.h>

#include "../hacks/misc.h"

void hooks::Setup() noexcept
{
	MH_Initialize();

	// AllocKeyValuesMemory hook
	MH_CreateHook(
		memory::Get(interfaces::keyValuesSystem, 1),
		&AllocKeyValuesMemory,
		reinterpret_cast<void**>(&AllocKeyValuesMemoryOriginal)
	);

	// CreateMove hook
	MH_CreateHook(
		memory::Get(interfaces::clientMode, 24),
		&CreateMove,
		reinterpret_cast<void**>(&CreateMoveOriginal)
	);

		//Paint traverse hook

	MH_CreateHook(
		memory::Get(interfaces::panel, 41),
		&PaintTraverse,
		reinterpret_cast<void**>(&PaintTraverseOriginal)
	);
		
	
	
	

	MH_EnableHook(MH_ALL_HOOKS);
}

void hooks::Destroy() noexcept
{
	// restore hooks
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);

	// uninit minhook
	MH_Uninitialize();
}

void* __stdcall hooks::AllocKeyValuesMemory(const std::int32_t size) noexcept
{
	// if function is returning to speficied addresses, return nullptr to "bypass"
	if (const std::uint32_t address = reinterpret_cast<std::uint32_t>(_ReturnAddress());
		address == reinterpret_cast<std::uint32_t>(memory::allocKeyValuesEngine) ||
		address == reinterpret_cast<std::uint32_t>(memory::allocKeyValuesClient)) 
		return nullptr;

	// return original
	return AllocKeyValuesMemoryOriginal(interfaces::keyValuesSystem, size);
}

bool __stdcall hooks::CreateMove(float frameTime, CUserCmd* cmd) noexcept
{
	// make sure this function is being called from CInput::CreateMove
	if (!cmd->commandNumber)
		return CreateMoveOriginal(interfaces::clientMode, frameTime, cmd);

	// this would be done anyway by returning true
	if (CreateMoveOriginal(interfaces::clientMode, frameTime, cmd))
		interfaces::engine->SetViewAngles(cmd->viewAngles);

	// get our local player here
	globals::UpdateLocalPlayer();

	if (globals::localPlayer && globals::localPlayer->IsAlive())
	{
		// example bhop
		hacks::RunBunnyHop(cmd);
	}

	return false;
}

void __stdcall hooks::PaintTraverse(std::uintptr_t vguiPanel, bool forceRepaint, bool allowForce) noexcept

{
	// make sure we have the correct panel
	if (vguiPanel == interfaces::engineVGui->GetPanel(PANEL_TOOLS))
	{
		//make sure local player is valid and we are in-game

		if (globals::localPlayer && interfaces::engine->IsInGame())
		{
			for (int i = 1; i < interfaces::globals->maxClients; ++i)
			{
				//get the player pointer

				CEntity* player = interfaces::entityList->GetEntityFromIndex(i);

				// make sure the player is valid

				if (!player)
					continue;

				// make sure player is not valid

				if (player->IsDormant() || !player->IsAlive())
					continue;

				// no ESP on team

				if (player->GetTeam() == globals::localPlayer->GetTeam())
					continue;

				// dont draw esp on dead player /spectating

				if(!globals::localPlayer->IsAlive())
					if(globals::localPlayer->GetObserverTarget() == player)
						continue;

				// players bone matrix

				CMatrix3x4 bones[128];
					if (!player->SetupBones(bones, 128, 0x7FF00, interfaces::globals->currentTime))
						continue;

					// screen postition of HEAD
					CVector top;

					if (interfaces::debugOverlay->ScreenPosition(bones[8].Origin() + CVector {0.f, 0.f, 11.f}, top))
						continue;

					// screen postition of Feet
					CVector bottom;

					if (interfaces::debugOverlay->ScreenPosition(player[8].GetAbsOrigin() - CVector{ 0.f, 0.f, 9.f }, bottom))
						continue;

					// height of box

					const float h = bottom.y - top.y;

					//  use the height to determine width

					const float w = h * 0.3f;

					const auto left = static_cast<int>(top.x - w);
					const auto right = static_cast<int>(top.x + w);


					


					// setting the color of the box

					interfaces::surface->DrawSetColor(240, 0, 255, 0);

					
					// draw normal box

					interfaces::surface->DrawOutlinedRect(left, top.y, right, bottom.y);
					
					
					// setting the outline color

					interfaces::surface->DrawSetColor(0, 4, 255, 0); 
					

					// draw the outlines

					interfaces::surface->DrawOutlinedRect(left - 1, top.y - 1, right - 1, bottom.y - 1);
					interfaces::surface->DrawOutlinedRect(left + 1, top.y + 1, right + 1, bottom.y + 1);
					

			
			}

		}

	}

	// call the orig function

	PaintTraverseOriginal(interfaces::panel, vguiPanel, forceRepaint, allowForce);
}
