//
// This is a generated file, do not change it manually.
// Use the script "generate_train_specification.py" instead.
//

#include "train_specification.h"

#include <game/train.h>

namespace resl {

/* 1d32:0000 : 140 bytes */
const TrainSpecification g_trainSpecifications[14] = {
    { // Server
        1800, 2000, 5,
        {
            CarriageType::Server,
            CarriageType::Server,
            CarriageType::Server,
            CarriageType::Server,
            CarriageType::Server,
        },
    },
    { // AncientLocomotive
        1800, 1850, 3,
        {
            CarriageType::AncientPassengerCarriage,
            CarriageType::OpenFreightCarriage,
            CarriageType::PocketWagon,
            CarriageType::AncientPassengerCarriage,
            CarriageType::AncientPassengerCarriage,
        },
    },
    { // SteamLocomotive
        1820, 1950, 4,
        {
            CarriageType::PassengerCarriage,
            CarriageType::OpenFreightCarriage,
            CarriageType::CoveredFreightCarriage,
            CarriageType::PocketWagon,
            CarriageType::TankWagon,
        },
    },
    { // Trolley
        1875, 1940, 5,
        {
            CarriageType::AncientPassengerCarriage,
            CarriageType::AncientPassengerCarriage,
            CarriageType::AncientPassengerCarriage,
            CarriageType::PassengerCarriage,
            CarriageType::PassengerCarriage,
        },
    },
    { // DieselLocomotive
        1929, 1970, 6,
        {
            CarriageType::PassengerCarriage,
            CarriageType::OpenFreightCarriage,
            CarriageType::CoveredFreightCarriage,
            CarriageType::PocketWagon,
            CarriageType::TankWagon,
        },
    },
    { // ElectricLocomotive
        1920, 1993, 6,
        {
            CarriageType::PassengerCarriage,
            CarriageType::OpenFreightCarriage,
            CarriageType::CoveredFreightCarriage,
            CarriageType::PocketWagon,
            CarriageType::TankWagon,
        },
    },
    { // HighSpeedLocomotive
        1975, 2000, 7,
        {
            CarriageType::HighSpeedPassengerCarriage,
            CarriageType::HighSpeedPassengerCarriage,
            CarriageType::HighSpeedPassengerCarriage,
            CarriageType::HighSpeedPassengerCarriage,
            CarriageType::HighSpeedPassengerCarriage,
        },
    },
    { // AncientPassengerCarriage
        1800, 1935, 0,
        {
            CarriageType::AncientLocomotive,
            CarriageType::SteamLocomotive,
            CarriageType::AncientLocomotive,
            CarriageType::SteamLocomotive,
            CarriageType::Trolley,
        },
    },
    { // PassengerCarriage
        1850, 1993, 0,
        {
            CarriageType::SteamLocomotive,
            CarriageType::ElectricLocomotive,
            CarriageType::DieselLocomotive,
            CarriageType::ElectricLocomotive,
            CarriageType::DieselLocomotive,
        },
    },
    { // HighSpeedPassengerCarriage
        1975, 2000, 0,
        {
            CarriageType::HighSpeedLocomotive,
            CarriageType::HighSpeedLocomotive,
            CarriageType::HighSpeedLocomotive,
            CarriageType::HighSpeedLocomotive,
            CarriageType::HighSpeedLocomotive,
        },
    },
    { // OpenFreightCarriage
        1800, 1950, 0,
        {
            CarriageType::SteamLocomotive,
            CarriageType::ElectricLocomotive,
            CarriageType::DieselLocomotive,
            CarriageType::ElectricLocomotive,
            CarriageType::DieselLocomotive,
        },
    },
    { // CoveredFreightCarriage
        1830, 1993, 0,
        {
            CarriageType::SteamLocomotive,
            CarriageType::ElectricLocomotive,
            CarriageType::DieselLocomotive,
            CarriageType::ElectricLocomotive,
            CarriageType::DieselLocomotive,
        },
    },
    { // PocketWagon
        1800, 1950, 0,
        {
            CarriageType::SteamLocomotive,
            CarriageType::ElectricLocomotive,
            CarriageType::DieselLocomotive,
            CarriageType::ElectricLocomotive,
            CarriageType::DieselLocomotive,
        },
    },
    { // TankWagon
        1830, 1993, 0,
        {
            CarriageType::SteamLocomotive,
            CarriageType::ElectricLocomotive,
            CarriageType::DieselLocomotive,
            CarriageType::ElectricLocomotive,
            CarriageType::DieselLocomotive,
        },
    },
};

} // namespace resl
