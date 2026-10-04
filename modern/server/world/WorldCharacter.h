#pragma once

// WORLD-ENTRY-001 Phase B: the authoritative character record.
//
// This is the ONLY source of a character's level, resources, map and position
// during world entry. The client names WHICH character to enter; it never says
// what that character is. Every value below is read from here and nowhere else.
//
// ---------------------------------------------------------------------------
// WHY THIS IS NOT modern/server/character/ServerCharacter
// ---------------------------------------------------------------------------
//
// ServerCharacter exists and is not reused, because it is a different thing. It
// owns derived-stat computation and equipment, skills, codex, status effects and
// combat - all of which depend on a class table, item definitions and skill
// definitions that WORLD-ENTRY-001 does not have and must not invent. It also has
// no map and no save position in the sense the protocol needs: its Vector3 is
// presentation state for combat, not `ChaSavePos`.
//
// Duplicating it here would be worse than the duplication the brief warns about,
// because a second character type would be a second answer to "what is this
// character's authoritative state", and the two would drift.
//
// ---------------------------------------------------------------------------
// WHAT IS HERE, AND WHY EACH FIELD
// ---------------------------------------------------------------------------
//
// Every field is one WORLD-ENTRY-001 puts on the wire. Nothing is here "for
// later": inventory, equipment, skills, guilds, quests and quickslots are out of
// scope, so they are absent rather than present-and-zero. Phase A already keeps
// their wire regions reserved and zero-filled; a field here would imply a
// subsystem that does not exist.
//
//   characterId   nChaNum. The DB's character key, and what 2353 carries.
//   accountId     nUserNum. The DB's account key. This is the OWNERSHIP key.
//   userId        szUserID, the login name the character belongs to.
//   name          m_szName / szChaName.
//   characterClass m_emClass / m_emCharClass. Raw on the wire; not interpreted.
//   school         m_wSchool.
//   level          m_wLevel. Authoritative; a client-supplied level is ignored.
//   hp/mp/sp       m_sHP / m_sMP / m_sSP, the GLDWDATA now/max union.
//   saveMapId      ChaSaveMap, read at s_COdbcGameChaGet.cpp:31 and :213.
//   savePosition   ChaSavePosX/Y/Z, :96 and :214-216, SQL_C_DOUBLE.
//   gaeaId         the ENTITY id every later gameplay message refers to.
//
// ---------------------------------------------------------------------------
// GAEAID IS AN ENTITY ID, NOT PERSISTENT DATA
// ---------------------------------------------------------------------------
//
// The brief lists gaeaId among the character record's fields, and it is here -
// but its semantics matter, because getting them wrong is how a re-entering
// character ends up with two live entities.
//
// In legacy the gaeaId is allocated by the FIELD server when it builds the GLChar,
// not read from the database: 2356 (Agent->Field) carries dwGaeaID = 0 and 2357
// (Field->Agent) is what returns the allocated one
// (NET_GAME_JOIN_FIELDSVR_FB::dwGaeaID, s_NetGlobal.h:4252). It is the handle for
// this one appearance of this character in the world.
//
// So `gaeaId == 0` means "not currently in the world", and it is assigned by the
// Field role through the repository at entry time. It is deliberately NOT
// persisted, and a fresh authorization always allocates a NEW one - which is what
// makes a stale authorization detectable rather than accidentally valid.
//
// The copy held by a pending world-entry authorization is a deliberate SNAPSHOT,
// not a second home: it is what 2359 is checked against, so an authorization issued
// for a previous appearance cannot admit the current one.

#include "CharacterListProtocol.h"
#include "NetworkTypes.h"
#include "RanWirePrimitives.h"
#include "WorldEntryProtocol.h"
#include "types/Result.h"

#include <cmath>
#include <string>

namespace Modern::Server::World
{
	// The account key. `nUserNum` in legacy, and the `UserNum=%d` half of the
	// ownership predicate. Typed rather than a bare WireU32 so that an account id
	// and a character id cannot be passed to each other's parameters.
	struct WorldAccountId
	{
		Network::WireU32 value = 0;

		friend bool operator==(const WorldAccountId& a, const WorldAccountId& b) noexcept
		{
			return a.value == b.value;
		}
		friend bool operator!=(const WorldAccountId& a, const WorldAccountId& b) noexcept
		{
			return a.value != b.value;
		}
	};

	// The character key. `nChaNum` in legacy, and the `ChaNum=%d` half of the
	// ownership predicate, and the value 2353 carries.
	struct WorldCharacterId
	{
		Network::WireU32 value = 0;

		friend bool operator==(const WorldCharacterId& a, const WorldCharacterId& b) noexcept
		{
			return a.value == b.value;
		}
		friend bool operator!=(const WorldCharacterId& a, const WorldCharacterId& b) noexcept
		{
			return a.value != b.value;
		}
	};

	// The authoritative character.
	//
	// A plain value with no behaviour, deliberately. Every rule that could live
	// here - clamping a resource, deriving a stat - either belongs to a subsystem
	// this milestone excludes or is already owned elsewhere. What this type owes
	// the reader is the guarantee that these numbers are the server's.
	struct WorldCharacter
	{
		WorldCharacterId id;
		WorldAccountId   accountId;
		std::string      userId;      // at most kUserIdFieldSize-1 characters
		std::string      name;        // at most kNameFieldSize-1 characters
		Network::WireU32     characterClass = 0; // raw; not interpreted here
		Network::WireU16     school         = 0;
		Network::WireU16     level          = 0;

		Network::RanWire::DwPair hp;
		Network::RanWire::DwPair mp;
		Network::RanWire::DwPair sp;

		// Where this character last was. §4.5 of the investigation: the spawn point
		// is the DB save position, and the school start point is only a fallback for
		// an unresolvable save map, a bRestart map, a dead character or a PVP-event
		// map. None of those conditions is modelled here, so this IS the spawn
		// point - and saying so is more honest than pretending a fallback exists.
		Network::RanWire::NativeId saveMapId;
		Network::RanWire::Vector3  savePosition;

		// The entity id for the current appearance; 0 when not in the world. See the
		// note above - allocated by the Field role, never persisted.
		Network::WireU32 gaeaId = 0;

		// WORLD-ENTRY-002a: the movement STATE half only. A bitmask, never an enum -
		// see MovementStateProtocol.h for why, and for which four bits a client may
		// influence. No coordinate is affected by any of this.
		Network::WireU32 actState = 0;

		// m_dwUserLvl: the ACCOUNT privilege level, carried by the Agent->Field join
		// (NET_GAME_JOIN_FIELDSVR::dwUserLvl, s_NetGlobal.h:4174) and by the client's
		// own join payload (GLContrlCharJoinMsg.h:87).
		//
		// Distinct from `level` above, which is the CHARACTER level, and the two must
		// not be conflated: the USER_GM3 gate that controls visibility flags is an
		// ACCOUNT check (USER_GM3 = 20, s_NetGlobal.h:313), and using a character's
		// level for it would let a levelled character set GM flags.
		//
		// 0 for an ordinary account, which is below USER_GM3 - the correct default.
		Network::WireU32 accountLevel = 0;

		// A field whose length would overflow the wire field, or a character name
		// that would not fit, is a repository construction error - refused at Add so
		// that no later stage has to defend against a record that cannot be encoded.
		//
		// Returned rather than clamped: a silently shortened character name is a
		// WRONG name, and a wrong name is worse than a refused character.
		Status Validate() const;

		friend bool operator==(const WorldCharacter& a, const WorldCharacter& b);
	};

	namespace Limits
	{
		// The field widths, taken from the Phase A wire definitions rather than
		// restated, so a protocol change moves this bound with it.
		constexpr std::size_t kMaxUserIdLength = Network::WorldEntry::kUserIdFieldSize - 1;
		constexpr std::size_t kMaxNameLength   = Network::CharacterList::kNameFieldSize - 1;

		static_assert(Network::WorldEntry::kUserIdFieldSize == 21,
		              "USR_ID_LENGTH + 1 is 21 bytes on the wire");
		static_assert(kMaxUserIdLength == 20, "so 20 characters fit with a terminator");
		static_assert(kMaxNameLength == 32, "CHAR_SZNAME is 33 bytes with a terminator");
	}
}