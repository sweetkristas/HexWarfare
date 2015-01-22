#include "creature.hpp"
#include "units.hpp"

namespace game
{
	unit::unit(const std::string& name, const creature::const_creature_ptr& cp, const player_ptr& owner, const uuid::uuid& id)
		: pos_(),
		  uuid_(id),
		  owner_(owner),
		  health_(1),
		  attack_(1),
		  armour_(0),
		  move_(1),
		  initiative_(100.0f),
		  name_(name),
		  range_(1),
		  critical_strike_(0.05f),
		  attacks_this_turn_(1),
		  type_(cp)
	{
	}

	unit_ptr unit::clone(const player_ptr& new_owner)
	{
		auto u = new unit(*this);
		u->owner_ = new_owner;
		return unit_ptr(u);
	}

	std::ostream& operator<<(std::ostream& os, const unit_ptr& u)
	{
		std::string uuid_short = uuid::write(u->get_uuid()).substr(0,5);
		os << "Unit(\"" << u->get_name() << "\",\"" << uuid_short 
			<< "\" H:" << u->get_health()
			<< " A:" << u->get_attack()
			<< " R:" << u->get_range()
			<< " M:" << u->get_move()
			<< " P:" << u->get_position()
			<< ")";
		return os;
	}

	void unit::start_turn(Update_UnitStats* uus)
	{
		// This could be things like healing at the start of the turn
		// And should definitely included a scripted component.
	}

	void unit::complete_turn(Update_UnitStats* uus)
	{
		// reset the movement for the unit at the front of the list.
		move_ = type_->get_movement();
		// reset the attacks per turn
		attacks_this_turn_ = type_->get_attacks_per_turn();
		// update the unit at the front of the list initiative.
		initiative_ += 100.0f / type_->get_initiative();
		// XXX add more things as required here to complete the units turn.

		uus->set_move(move_);
		uus->set_attacks_this_turn(attacks_this_turn_);
		uus->set_initiative(initiative_);
	}

	player_ptr unit::get_owner() const
	{ 
		auto o = owner_.lock(); 
		ASSERT_LOG(o != nullptr, "Couldn't find owner for unit "); 
		return o; 
	}
}