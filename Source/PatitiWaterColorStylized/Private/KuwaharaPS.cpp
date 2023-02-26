#include "KuwaharaPS.h"

IMPLEMENT_GLOBAL_SHADER(FKuwaharaVS, "/PatitiStylized/Private/Kuwahara.usf", "KuwaharaVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FKuwaharaPS, "/PatitiStylized/Private/Kuwahara.usf", "KuwaharaPS", SF_Pixel);