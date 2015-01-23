#include "pnstdio.h"
#include "pnobject.h"
#include "pnfunction.h"
#include "pnstring.h"
#include <string.h>
#include <stdlib.h>


static void __print(char *s)
{
    // fputs(s, stdout); // <-- this has different buffers than printf, switch order.. -_-;;
    printf("%s", s);
}

static pn_object *PnStdio_Print(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(length == 1);

    pn_object *a_param = NULL;

    if (IS_STRING(params[0]))
        a_param = params[0];
    else
        a_param = PnFunction_ExecuteByObject("to_str", world, params[0], NULL, 0);

    PN_ASSERT(IS_STRING(a_param));

    __print(a_param->val.str_val);

    pn_object *ret = PnString_Create(world, a_param->val.str_val);
    PN_ASSERT(ret != NULL);
    PN_ASSERT(IS_STRING(ret));
    return ret;
}

static pn_object *PnStdio_PrintLine(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(length == 1);

    pn_object *a_param = NULL;

    if (IS_STRING(params[0]))
        a_param = params[0];
    else
        a_param = PnFunction_ExecuteByObject("to_str", world, params[0], NULL, 0);

    PN_ASSERT(IS_STRING(a_param));

    __print(a_param->val.str_val);
    __print("\n");

    size_t len = strlen(a_param->val.str_val);
    char *s = (char *)malloc(sizeof(char) * (len + 2));
    strcpy(s, a_param->val.str_val);
    s[len] = '\n';
    s[len+1] = 0;

    pn_object *ret = PnString_Create(world, s);
    PN_ASSERT(ret != NULL);
    PN_ASSERT(IS_STRING(ret));

    free(s);

    return ret;
}

static pn_object *PnStdio_ReadLine(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    static char buffer[INPUT_BUFFER_SIZE];

    pn_object *ret = NULL;
    if (fgets(buffer, INPUT_BUFFER_SIZE, stdin) != NULL) {
        ret = PnString_Create(world, buffer);
    } else {
        // FIXME throw exeception?
        ret = PnNull_Create(world);
    }

    return ret;
}

pn_object *PnStdio_CreatePrototype(pn_world *world)
{
    pn_object *o = PnObject_CreateEmptyObject(world);
    o->type = TYPE_OBJECT;

    PnObject_PutAttr(world, o, "print", PnFunction_CreateByNative(world, PnStdio_Print));
    PnObject_PutAttr(world, o, "printline", PnFunction_CreateByNative(world, PnStdio_PrintLine));
    PnObject_PutAttr(world, o, "readline", PnFunction_CreateByNative(world, PnStdio_ReadLine));

    return o;
}

