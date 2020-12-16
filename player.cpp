#include "player.h"

Player::Player(unsigned int uuid, QString name, QString country)
{
	this->UUID = uuid;
	this->name = name;
	this->country = country;
	this->score = 0;
}

void Player::setUUID(unsigned int param)
{
	this->UUID = param;
}

void Player::setName(QString param)
{
	this->name = param;
}

void Player::setCountry(QString param)
{
	this->country = param;
}

void Player::setScore(int param)
{
	this->score = param;
}
