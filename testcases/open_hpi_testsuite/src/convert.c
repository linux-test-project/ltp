/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by FORCE Computers
 *
 * Note that this file is based on parts of OpenIPMI
 * written by Corey Minyard <minyard@mvista.com>
 * of MontaVista Software. Corey's code was helpful
 * and many thanks go to him. He gave the permission
 * to use this code in OpenHPI.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Thomas Kanngieser <thomas.kanngieser@fci.com>
 */


#include <math.h>

#include <SaHpi.h>
#include <openhpi.h>


static double
c_linear(double val)
{
        return val;
}


static double
c_exp10(double val)
{
        return pow(10.0, val);
}


static double
c_exp2(double val)
{
        return pow(2.0, val);
}


static double
c_1_over_x(double val)
{
        return 1.0 / val;
}


static double
c_sqr(double val)
{
        return pow(val, 2.0);
}


static double
c_cube(double val)
{
        return pow(val, 3.0);
}


static double
c_1_over_cube(double val)
{
        return 1.0 / pow(val, 3.0);
}


typedef double (*linearizer)(double val);
static linearizer linearize[11] =
{
        c_linear,
        log,
        log10,
        exp,
        c_exp10,
        c_exp2,
        c_1_over_x,
        c_sqr,
        c_cube,
        sqrt,
        c_1_over_cube
};


static int
sign_extend(int m, int bits)
{
        if (m & (1 << (bits-1)))
                return m | (-1 << bits);
        else
                return m & (~(-1 << bits));
}


/**
 * sensor_convert_from_raw: convert raw sensor values to interpreted values.
 * It can be used to convert raw data, raw thresholds or raw hysteresis of a sensor
 * to human readable format.
 *
 * @sensor: sensor.
 * @val: raw sensor value.
 * @result: the interpreted value
 *
 * Return value: SA_OK if conversion possible
 **/
SaErrorT 
sensor_convert_from_raw (SaHpiSensorRecT *sensor,
                         SaHpiUint32T     val,
                         SaHpiFloat32T   *result)
{
        SaHpiFloat32T m, b, b_exp, r_exp, fval;
        linearizer c_func;
        SaHpiSensorFactorsT *factors = &sensor->DataFormat.Factors;

        if ( factors->Linearization == SAHPI_SL_NONLINEAR )
                c_func = c_linear;
        else if ( factors->Linearization <= 11 )
                c_func = linearize[factors->Linearization];
        else
                return SA_ERR_HPI_INVALID_DATA;

        val &= 0xff;

        m     = (SaHpiFloat32T)factors->M_Factor;
        b     = (SaHpiFloat32T)factors->B_Factor;
        r_exp = (SaHpiFloat32T)factors->ExpR;
        b_exp = (SaHpiFloat32T)factors->ExpB;

        switch( sensor->DataFormat.SignFormat ) {
        case SAHPI_SDF_UNSIGNED:
                fval = (SaHpiFloat32T)val;
                break;

        case SAHPI_SDF_1S_COMPLEMENT:
                val = sign_extend( val, 8 );
                if ( val < 0 )
                        val += 1;

                fval = (SaHpiFloat32T)val;
                break;

        case SAHPI_SDF_2S_COMPLEMENT:
                fval = (SaHpiFloat32T)sign_extend( val, 8 );
                break;

        default:
                return SA_ERR_HPI_INVALID_DATA;
        }

        *result = c_func(   ((m * fval) + (b * pow(10, b_exp)))
                          * pow(10, r_exp) );

        return SA_OK;
}


/**
 * sensor_convert_to_raw: convert interpreted sensor values to raw values.
 * It can be used to convert data, thresholds or hysteresis of a sensor
 * to raw values. The raw values are in the range of [0-255]. The function
 * use binary search to get the raw value.
 *
 * @sensor: sensor.
 * @val: interpreted sensor value.
 * @result: raw value
 *
 * Return value: SA_OK if convertion possible
 **/
SaErrorT
sensor_convert_to_raw (SaHpiSensorRecT *sensor,
                       SaHpiFloat32T    val,
                       SaHpiUint32T    *result)
{
        SaHpiFloat32T cval;
        int    lowraw, highraw, raw, maxraw, minraw, next_raw;
        int    rv;

        switch(sensor->DataFormat.SignFormat) {
        case SAHPI_SDF_UNSIGNED:
                lowraw   =   0;
                highraw  = 255;
                minraw   =   0;
                maxraw   = 255;
                next_raw = 128;
                break;

        case SAHPI_SDF_1S_COMPLEMENT:
                lowraw   = -127;
                highraw  =  127;
                minraw   = -127;
                maxraw   =  127;
                next_raw =    0;
                break;

        case SAHPI_SDF_2S_COMPLEMENT:
                lowraw   = -128;
                highraw  =  127;
                minraw   = -128;
                maxraw   =  127;
                next_raw =    0;
                break;

        default:
                return SA_ERR_HPI_INVALID_DATA;
        }

        /* We do a binary search for the right value.  Yuck, but I don't
           have a better plan that will work with non-linear sensors. */
        do {
                raw = next_raw;
                rv = sensor_convert_from_raw(sensor, raw, &cval);
                if ( rv )
                        return rv;

                if (cval < val) {
                        next_raw = ((highraw - raw) / 2) + raw;
                        lowraw = raw;
                } else {
                        next_raw = ((raw - lowraw) / 2) + lowraw;
                        highraw = raw;
                }
        } while(raw != next_raw);

        /* The above loop gets us to within 1 of what it should be, we
           have to look at rounding to make the final decision. */

        if (val > cval) {
                if (raw < maxraw) {
                        SaHpiFloat32T nval;

                        rv = sensor_convert_from_raw(sensor, raw+1, &nval);
                        if (rv)
                                return rv;

                        nval = cval + ((nval - cval) / 2.0);
                        if (val >= nval)
                                raw++;
                }
        } else {
                if (raw > minraw) {
                        SaHpiFloat32T pval;

                        rv = sensor_convert_from_raw(sensor, raw-1, &pval);
                        if (rv)
                                return rv;

                        pval = pval + ((cval - pval) / 2.0);
                        if (val < pval)
                                raw--;
                }
        }

        if (sensor->DataFormat.SignFormat == SAHPI_SDF_1S_COMPLEMENT)
                if (raw < 0)
                        raw -= 1;

        *result = raw & 0xff;

        return SA_OK;
}
