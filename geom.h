/*
 * geometry macros
 */
#ifndef _GEOM_H_
#define _GEOM_H_

/*
 *  SQ      2DD
 *  SQRT    1DD     I2D
 *          2D
 *
 *  SQ - square
 *  SQRT - square root
 *
 *  2DD - 2D diff
 *  1DD - 1D diff
 *  2D  - 2D
 *
 *  I2D - int to double
 */

#define SQ_1DD_I2D(a1,a2) \
    (((double)(a2)-(double)(a1))*((double)(a2)-(double)(a1)))

#define SQRT_2DD_I2D(px1,py1,px2,py2) \
    sqrt( SQ_1DD_I2D(px1,px2) + SQ_1DD_I2D(py1,py2) )

#define SQRT_2D(px,py) \
    sqrt( px*px + py*py )

#endif /* _GEOM_H_ */
