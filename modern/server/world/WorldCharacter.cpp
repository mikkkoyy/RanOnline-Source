#include "world/WorldCharacter.h"

namespace Modern::Server::World
{
	Status WorldCharacter::Validate() const
	{
		// A zero id on either side would make the ownership predicate trivially
		// satisfiable, so it is refused rather than stored.
		if (id.value == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (accountId.value == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// An empty userId would make the ownership check depend on a string compare
		// against "", which every account would fail and no account would be able to
		// explain. An empty name is a character nobody could select.
		if (userId.empty() || userId.size() > Limits::kMaxUserIdLength)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (name.empty() || name.size() > Limits::kMaxNameLength)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// A position the encoder would turn into a non-finite float. RAN's save
		// columns are SQL_C_DOUBLE, so a NULL or NaN reaches this type in
		// principle; refusing it here means no spawn packet ever carries one.
		const float components[3] = { savePosition.x, savePosition.y, savePosition.z };
		for (const float component : components)
		{
			if (!std::isfinite(component))
			{
				return Status(ErrorCode::InvalidArgument);
			}
		}

		// Level 0 does not exist in RAN (the table starts at 1), and a resource
		// above its own maximum cannot be encoded honestly.
		if (level == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (hp.now > hp.max || mp.now > mp.max || sp.now > sp.max)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		return Ok();
	}

	bool operator==(const WorldCharacter& a, const WorldCharacter& b)
	{
		return a.id == b.id && a.accountId == b.accountId && a.userId == b.userId &&
		       a.name == b.name && a.characterClass == b.characterClass &&
		       a.school == b.school && a.level == b.level && a.hp.now == b.hp.now &&
		       a.hp.max == b.hp.max && a.mp.now == b.mp.now && a.mp.max == b.mp.max &&
		       a.sp.now == b.sp.now && a.sp.max == b.sp.max &&
		       a.saveMapId.value == b.saveMapId.value && a.gaeaId == b.gaeaId &&
		       a.savePosition.x == b.savePosition.x &&
		       a.savePosition.y == b.savePosition.y &&
		       a.savePosition.z == b.savePosition.z;
	}
}