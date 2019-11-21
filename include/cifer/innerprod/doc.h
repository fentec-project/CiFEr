/*
 * Copyright (c) 2018 XLAB d.o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \defgroup innerprod Inner product schemes
 * Functional encryption schemes for inner products.
 * Based on security assumptions, the schemes are organized into submodules
 * simple (s-IND-CPA security), and fullysec (e.g. "fully secure", offering
 * adaptive IND-CPA security).
 *
 * Note that in both modules you will find single input as well as multi input
 * schemes. Construction of all multi input schemes is based on the work of
 * Abdalla et. al (see paper: https://eprint.iacr.org/2017/972.pdf)
 *
 * Most of the functions write their results to pointers which are passed as
 * parameters. The memory used by these results needs to be freed by the caller
 * with *xyz_free* functions (each struct defined in this library has a respective
 * *struct_name_free* function). These free the memory used by the struct's
 * members, but not the memory used by the struct itself. This is due to the
 * initialization of the structs' members in functions such as *generate_keys*
 * and *derive_fe_key* - this makes the API simpler and easier to use as the user
 * of the library does not to know all struct fields.
 *
 * The results of the functions are the
 * parameters listed _before_ the pointer to the struct in the parameter list.
 * Consult the appropriate documentation for each scheme for more thorough
 * descriptions of functions and their results.
 *
 */