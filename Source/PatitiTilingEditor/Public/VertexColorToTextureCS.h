#pragma once
#include "GlobalShader.h"
#include <ShaderParameterStruct.h>
#include "ShaderCore.h"

#define ThreadGroupSize 1

struct STVerticesUV
{
	FVector2f UV;
};

struct STVerticesRGBA
{
	FVector4f RGBA;
};


class FVertexColorToTextureCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FVertexColorToTextureCS);
	SHADER_USE_PARAMETER_STRUCT(FVertexColorToTextureCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, OutputTexture)
		SHADER_PARAMETER_SRV(StructuredBuffer<FVector2>, InputTriangleVertexUVs)
		SHADER_PARAMETER_SRV(StructuredBuffer<FVector4>, InputVerticesRGBA)
	END_SHADER_PARAMETER_STRUCT()


		static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnviroment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnviroment);

		OutEnviroment.SetDefine(TEXT("ThreadGroupSizeX"), ThreadGroupSize);
		OutEnviroment.SetDefine(TEXT("ThreadGroupSizeY"), ThreadGroupSize);
		OutEnviroment.SetDefine(TEXT("ThreadGroupSizeZ"), 1);
	}
};