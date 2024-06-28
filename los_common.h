#pragma once

/*
** Lib4zip line of sight (los) codec common include file.
** Copyright (C) 2024 Julien Perrier-cornet
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cassert>
#include "arith.h"


static inline uint32_t los_probaGamble( uint32_t sum_1, uint32_t sum )
{
    if (sum_1 == 0) return 0;
    uint32_t p = (uint32_t) (((((uint64_t) sum_1) << ARITH_PRECISION) - (sum_1 << (ARITH_PRECISION-12))) / sum) + (1 << (ARITH_PRECISION-13));
    return p;
}


