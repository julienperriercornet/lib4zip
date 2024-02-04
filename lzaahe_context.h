#pragma once

/*
** Lib4zip lzaahe codec context header.
** Copyright (C) 2022-2024 Julien Perrier-cornet
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

#include "lib4zip.h"
#include "bitio.h"
#include "platform.h"



#define LZAAHE_REFHASH_BITS (LZAAHE_BLOCK_BITS)
#define LZAAHE_REFHASH_SZ (1<<LZAAHE_REFHASH_BITS)
#define LZAAHE_REFHASH_ENTITIES (2)
#define LZAAHE_MAX_SYMBOLS (1<<(LZAAHE_BLOCK_BITS-3))
#define LZAAHE_MAX_MATCHES (1<<(LZAAHE_BLOCK_BITS-2))

