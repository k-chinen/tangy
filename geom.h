#ifndef _GEOM_H_
#define _GEOM_H_

#define SQ_I2D(a1,a2) \
    (((double)(a2)-(double)(a1))*((double)(a2)-(double)(a1)))
#define SQRT_I2D(px1,py1,px2,py2) \
    sqrt( SQ_I2D(px1,px2) + SQ_I2D(py1,py2))

#endif /* _GEOM_H_ */
