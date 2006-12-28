#include <math.h>
#include <string.h>
#include "sp_vm_api.h"
#include "sp_vm_typeutil.h"

using namespace SourcePawn;

#define PI 3.14159265358979323846

inline float AngleToRadians(float val, int mode)
{
	switch (mode)
	{
	case 1:
		{
			return (float)((val * PI) / 180.0);
		}
	case 2:
		{
			return (float)((val * PI) / 200.0);
		}
	default:
		{
			return val;
		}
	}
}

inline float RadiansToAngle(float val, int mode)
{
	switch (mode)
	{
	case 1:
		{
			return (float)((val / PI) * 180.0);
		}
	case 2:
		{
			return (float)((val / PI) * 200.0);
		}
	default:
		{
			return val;
		}
	}
}


/****************************************
*                                       *
* FLOATING POINT NATIVE IMPLEMENTATIONS *
*                                       *
****************************************/


static cell_t sm_float(IPluginContext *pCtx, const cell_t *params)
{
	float val = static_cast<float>(params[1]);

	return ftoc(val);
}

static cell_t sm_floatabs(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]);
	val = (val >= 0) ? val : -val;

	return ftoc(val);
}

static cell_t sm_floatadd(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]) + ctof(params[2]);

	return ftoc(val);
}

static cell_t sm_floatsub(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]) - ctof(params[2]);

	return ftoc(val);
}

static cell_t sm_floatmul(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]) * ctof(params[2]);

	return ftoc(val);
}

static cell_t sm_floatdiv(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]) / ctof(params[2]);

	return ftoc(val);
}

static cell_t sm_floatcmp(IPluginContext *pCtx, const cell_t *params)
{
	float val1 = ctof(params[1]);
	float val2 = ctof(params[2]);

	if (val1 > val2)
	{
		return 1;
	} else if (val1 < val2) {
		return -1;
	}

	return 0;
}

static cell_t sm_floatlog(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]);
	float base = ctof(params[2]);

	if ((val <= 0) || (base <= 0))
	{
		//:TODO: error out! logs cant take in negative numbers and log 0=-inf
	}
	if (base == 10.0)
	{
		val = log10(val);
	} else {
		val = log(val) / log(base);
	}

	return ftoc(val);
}

static cell_t sm_floatexp(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]);

	return ftoc(exp(val));
}

static cell_t sm_floatpower(IPluginContext *pCtx, const cell_t *params)
{
	float base = ctof(params[1]);
	float exponent = ctof(params[2]);

	return ftoc(pow(base, exponent));
}

static cell_t sm_floatsqroot(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]);

	if (val < 0.0)
	{
		//:TODO: error out! we dont support complex numbers
	}

	return ftoc(sqrt(val));
}

static cell_t sm_floatround(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]);

	switch (params[2])
	{
	case 1:
		{
			val = floor(val);
			break;
		}
	case 2:
		{
			val = ceil(val);
			break;
		}
	case 3:
		{
			if (val >= 0.0)
			{
				val = floor(val);
			} else {
				val = ceil(val);
			}
			break;
		}
	default:
		{
			val = (float)floor(val + 0.5);
			break;
		}
	}

	return ftoc(val);
}

static cell_t sm_floatstr(IPluginContext *pCtx, const cell_t *params)
{
	char *str;

	pCtx->LocalToString(params[1], &str);
	if (!strlen(str))
	{
		return 0;
	}

	return ftoc((float)atof(str));
}

static cell_t sm_floatfract(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]);
	val = val - floor(val);

	return ftoc(val);
}

static cell_t sm_floatsin(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]);
	val = sin(AngleToRadians(val, params[2]));

	return ftoc(val);
}

static cell_t sm_floatcos(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]);
	val = cos(AngleToRadians(val, params[2]));

	return ftoc(val);
}

static cell_t sm_floattan(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]);
	val = tan(AngleToRadians(val, params[2]));

	return ftoc(val);
}

static cell_t sm_floatasin(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]);
	val = asin(val);

	return ftoc(RadiansToAngle(val, params[2]));
}

static cell_t sm_floatacos(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]);
	val = acos(val);

	return ftoc(RadiansToAngle(val, params[2]));
}

static cell_t sm_floatatan(IPluginContext *pCtx, const cell_t *params)
{
	float val = ctof(params[1]);
	val = atan(val);

	return ftoc(RadiansToAngle(val, params[2]));
}

static cell_t sm_floatatan2(IPluginContext *pCtx, const cell_t *params)
{
	float val1 = ctof(params[1]);
	float val2 = ctof(params[2]);
	val1 = atan2(val1, val2);

	return ftoc(RadiansToAngle(val1, params[3]));
}
