#ifndef ARPADICA_COUNTRY_H
#define ARPADICA_COUNTRY_H

#include "raylib.h"
#include "map_engine.hpp"

#include <string>
#include <iostream>
#include <cmath>
#include <cfloat>
#include <vector>

enum Ideology
{
	TOTALITARIAN_RIGHT,
	AUTHORITARIAN_RIGHT,
	DEMOCRATIC_RIGHT,
	NONALIGNED,
	DEMOCRATIC_LEFT,
	AUTHORITARIAN_LEFT,
	TOTALITARIAN_LEFT
};

class Country
{
	private:
		std::string id; // 3 letter id
		vector<std::string> state_ids; // the states that belong to this country

		Color color;

		std::string name_totalitarian_right;
		std::string name_authoritarian_right;
		std::string name_democratic_right;
		std::string name_nonaligned;
		std::string name_democratic_left;
		std::string name_authoritarian_left;
		std::string name_totalitarian_left;

		Texture2D flag_totalitarian_right;
		Texture2D flag_authoritarian_right;
		Texture2D flag_democratic_right;
		Texture2D flag_nonaligned;
		Texture2D flag_democratic_left;
		Texture2D flag_authoritarian_left;
		Texture2D flag_totalitarian_left;

	public:
		Country(const std::string& country_id, Color color) : id(country_id), color(color) {}

		Country(const std::string& country_id, Color color, const std::vector<std::string>& states) : id(country_id), color(color), state_ids(states) {}

		std::string getId() const { return id; }

		void addStateId(const std::string& state_id)
		{
			state_ids.push_back(state_id);
		}
		void removeStateId(const std::string& state_id)
		{
			state_ids.erase(std::remove(state_ids.begin(), state_ids.end(), state_id), state_ids.end());
		}
		const std::vector<std::string>& getStateIds() const { return state_ids; }


		// NAMES
		void setNames(const std::string& totalitarian_right, const std::string& authoritarian_right, const std::string& democratic_right, const std::string& nonaligned, const std::string& democratic_left, const std::string& authoritarian_left, const std::string& totalitarian_left)
		{
			name_totalitarian_right = totalitarian_right;
			name_authoritarian_right = authoritarian_right;
			name_democratic_right = democratic_right;
			name_nonaligned = nonaligned;
			name_democratic_left = democratic_left;
			name_authoritarian_left = authoritarian_left;
			name_totalitarian_left = totalitarian_left;
		}

		std::string getName(Ideology ideology) const
		{
			switch(ideology)
			{
				case TOTALITARIAN_RIGHT: return name_totalitarian_right;
				case AUTHORITARIAN_RIGHT: return name_authoritarian_right;
				case DEMOCRATIC_RIGHT: return name_democratic_right;
				case NONALIGNED: return name_nonaligned;
				case DEMOCRATIC_LEFT: return name_democratic_left;
				case AUTHORITARIAN_LEFT: return name_authoritarian_left;
				case TOTALITARIAN_LEFT: return name_totalitarian_left;
				default: return "";
			}
		}

		// FLAGS
		void setFlags(const Texture2D& totalitarian_right, const Texture2D& authoritarian_right, const Texture2D& democratic_right, const Texture2D& nonaligned, const Texture2D& democratic_left, const Texture2D& authoritarian_left, const Texture2D& totalitarian_left)
		{
			flag_totalitarian_right = totalitarian_right;
			flag_authoritarian_right = authoritarian_right;
			flag_democratic_right = democratic_right;
			flag_nonaligned = nonaligned;
			flag_democratic_left = democratic_left;
			flag_authoritarian_left = authoritarian_left;
			flag_totalitarian_left = totalitarian_left;
		}

		Texture2D getFlag(Ideology ideology) const
		{
			switch(ideology)
			{
				case TOTALITARIAN_RIGHT: return flag_totalitarian_right;
				case AUTHORITARIAN_RIGHT: return flag_authoritarian_right;
				case DEMOCRATIC_RIGHT: return flag_democratic_right;
				case NONALIGNED: return flag_nonaligned;
				case DEMOCRATIC_LEFT: return flag_democratic_left;
				case AUTHORITARIAN_LEFT: return flag_authoritarian_left;
				case TOTALITARIAN_LEFT: return flag_totalitarian_left;
				default: return Texture2D{};
			}
		}

		Color getColor() const { return color; }




};

#endif