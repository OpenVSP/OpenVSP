# The USAF 1966 (Herrington) atmosphere.  CalcAtmosphere takes meters, K and gives kPa.

import openvsp as vsp


def testUSAF1966TemperatureOffsetKeepsThePressure():
    """The offset was scaled with altitude, and pressure and density ignored it."""
    alt = 5000.0
    t0, p0, pr0, rr0 = vsp.CalcAtmosphere( alt, 0.0, vsp.ATMOS_TYPE_HERRINGTON_1966 )
    t1, p1, pr1, rr1 = vsp.CalcAtmosphere( alt, 20.0, vsp.ATMOS_TYPE_HERRINGTON_1966 )
    assert abs( ( t1 - t0 ) - 20.0 ) < 1e-9, ( t0, t1 )
    assert abs( p1 - p0 ) < 1e-9, ( p0, p1 )
    assert abs( rr1 / rr0 - t0 / t1 ) < 1e-9, ( rr0, rr1, t0, t1 )


def testUSAF1966IsContinuousAtTheTropopause():
    """36089 ft fell through both layer tests and took the 82021 ft values."""
    below = vsp.CalcAtmosphere( 36088.9 * 0.3048, 0.0, vsp.ATMOS_TYPE_HERRINGTON_1966 )[1]
    at = vsp.CalcAtmosphere( 36089.0 * 0.3048, 0.0, vsp.ATMOS_TYPE_HERRINGTON_1966 )[1]
    assert abs( at / below - 1.0 ) < 1e-3, ( below, at )
