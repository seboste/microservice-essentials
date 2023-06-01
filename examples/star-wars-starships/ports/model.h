#pragma once

#include <string>

enum class StarshipStatus
{
  Unknown,
  UnderConstruction,
  OnStandby,
  InAction,
  Damaged,
  Destroyed
};

struct StarshipProperties
{
  std::string Id;
  std::string Name;
  std::string Description;
};

struct Starship
{
  StarshipProperties Properties;
  StarshipStatus Status;
};
