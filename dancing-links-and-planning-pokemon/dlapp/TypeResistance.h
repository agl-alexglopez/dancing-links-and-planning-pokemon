#ifndef TYPERESISTANCE_H
#define TYPERESISTANCE_H
#include "Src/Resistance.h"

namespace DancingLinks {
class Resistance;

/**
 * @brief type  returns the TypeEncoding for the resistance. May be viewed as a pair of strings.
 * @param res   the Resistance instance.
 * @return      the TypeEncoding, an efficient representation of a type. May be printed as a string.
 */
TypeEncoding type(const Resistance& res);

/**
 * @brief multiplier  returns the multiplier for a Resistance object.
 * @param res         the instance of a Resistance.
 * @return            EMPTY_,IMMUNE(x0),FRAC14(x.25),FRAC12(x.5),NORMAL(x1),DOUBLE(x2) or QUADRU(x4)
 */
Multiplier multiplier(const Resistance& res);


} // namespace DancingLinks

#endif // TYPERESISTANCE_H
