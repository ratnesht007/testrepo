#pragma once

#include<iostream>


typedef struct cdm_cash_unit
{
    int Type;
    char UnitID[5];
    short denominations;
    int count;
    int maximum;
    short Status;

    cdm_cash_unit() : Type(0), UnitID{ 0 }, denominations(0), count(0), maximum(0), Status(0) {

    }

    cdm_cash_unit(const cdm_cash_unit& other):Type(other.Type), denominations(other.denominations), count(other.count), maximum(other.maximum), Status(other.Status) {

        strcpy_s(UnitID, other.UnitID);

    }

    cdm_cash_unit(int type, const char* unitid, short denom, int cnt, int max, short st)
        : Type(type), denominations(denom), count(cnt), maximum(max), Status(st)
    {
        strcpy_s(UnitID, unitid);
    }

    ~cdm_cash_unit() {
        std::cout << "cdm_cash_unit deleted\n";
    }

} CASHUNIT, * LPCASHUNIT;

typedef struct cdm_cu_info
{
    int count;
    LPCASHUNIT* lppList;

    ~cdm_cu_info() {
        std::cout << "cdm_cu_info deleted\n";
    }

} CUINFO, * LPCUINFO;
