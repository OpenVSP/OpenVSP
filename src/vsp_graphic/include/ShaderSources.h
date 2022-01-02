#ifndef _VSP_GRAPHIC_GLSL_SOURCE_H
#define _VSP_GRAPHIC_GLSL_SOURCE_H

namespace VSPGraphic
{
/*!
* Shader Source Code for rendering a single texture with no lighting.
*
* vShader_Texture01 transforms texture's coordinate.
*
* fShader_Texture01 calculates texture color.
*/
static const char * const vShader_Texture01_No_Lighting =
    "#version 120 \n"

    "void main()"
    "{"
    "	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"

    "	gl_FrontColor = gl_Color;"

    "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
    "}";

static const char * const fShader_Texture01_No_Lighting =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[1];"
    "uniform vec2 u_Scales[1];"
    "uniform vec2 u_FlipFlags[1];"
    "uniform float u_TexAlphas[1];"

    "void main()"
    "{"
    "	vec4 resultColor;"

    // To avoid potential bugs, using uniform control flow as much as possible.

    // iBoundCoord is the Texture Coordinate within 0 and 1.
    "	vec2 iBoundCoord = gl_TexCoord[0].st;"

    // oBoundCoord is the Texture Coordinate outside 0 and 1 range.
    // Texture Map T Translate:
    // |----------| 2
    // |----------| 3
    // |----------| 4  <- 1.0
    // |          |
    // |          |
    // |----------| 0  <- (Scale)
    // |----------| 1  <- (Current TexCoord - Scale)
    "	vec2 oBoundCoord = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"

    // Flip S Coordinate if needed.
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord.s = 1 - iBoundCoord.s;"
    "		oBoundCoord.s = 1 - oBoundCoord.s;"
    "	}"

    // Flip T Coordinate if needed.
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord.t = 1 - iBoundCoord.t;"
    "		oBoundCoord.t = 1 - oBoundCoord.t;"
    "	}"

    // iBoundColor and oBoundColor are used to create smooth translation cross texture boundary.
    "	vec4 iBoundColor = texture2D(u_SamplerIds[0], iBoundCoord);"
    "	vec4 oBoundColor = texture2D(u_SamplerIds[0], oBoundCoord);"

    // Set Color base on the coordinate of texture.
    "	resultColor = iBoundColor;"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		resultColor = oBoundColor;"
    "	}"

    // Blend texture to geometry with appropriate alpha value.
    // Alpha value is calculate base on current location of texture. If texture
    // is on border(alpha equals 0), use alpha 0, else use alpha value from u_TexAlphas.
    "	resultColor = mix(gl_Color, resultColor, min(resultColor.a, u_TexAlphas[0]));"

    "	gl_FragColor = resultColor;"
    "}";

/*!
* Shader Source Code for rendering a single texture.
*
* vShader_Texture01 calculates lighting color from up to eight light sources,
* and transforms texture's coordinate.
*
* fShader_Texture01 calculates texture color.
*/
static const char * const vShader_Texture01 =
    "#version 120 \n"

    "uniform vec4 u_LightSwitches[8];"

    "varying vec4 v_LightColor;"

    // Calculate ambient, diffuse and specular for each light source.

    // Light source 0.
    // Vertex Normal and Global Ambient are same to all light source.
    // To maximize performance, only calculate them once and pass as
    // parameters.
    "vec4 light0(vec3 normal, vec4 globalAmbient)"
    "{"
    //  Light source direction.
    "	vec3 lightDir = normalize(gl_LightSource[0].position.xyz);"
    // Find Half Vector of reflected light.
    "	vec3 halfVector = gl_LightSource[0].halfVector.xyz;"

    // Calculate ambient, diffuse, and specular lightings.
    "	vec4 ambient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[0].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"

    // Return color of light source 0.
    "	return ambient + diffuse + specular;"
    "}"

    // Light source 1.
    "vec4 light1(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[1].position.xyz);"
    "	vec3 halfVector = gl_LightSource[1].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[1].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[1].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[1].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    // Light source 2.
    "vec4 light2(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[2].position.xyz);"
    "	vec3 halfVector = gl_LightSource[2].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[2].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[2].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[2].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    // Light source 3.
    "vec4 light3(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[3].position.xyz);"
    "	vec3 halfVector = gl_LightSource[3].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[3].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[3].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[3].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    // Light source 4.
    "vec4 light4(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[4].position.xyz);"
    "	vec3 halfVector = gl_LightSource[4].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[4].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[4].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[4].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    // Light source 5.
    "vec4 light5(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[5].position.xyz);"
    "	vec3 halfVector = gl_LightSource[5].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[5].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[5].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[5].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    // Light source 6.
    "vec4 light6(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[6].position.xyz);"
    "	vec3 halfVector = gl_LightSource[6].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[6].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[6].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[6].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    // Light source 7.
    "vec4 light7(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[7].position.xyz);"
    "	vec3 halfVector = gl_LightSource[7].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[7].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[7].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[7].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    // Calculate Combined Light Color.
    "vec4 calculateLightColor()"
    "{"
    "	vec4 color;"

    //  Calculate Vertex Normal and Global Ambient.
    "	vec3 normal = normalize(gl_NormalMatrix * gl_Normal);"
    "	vec4 globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;"

    //  Enable or disable light by multiply light color with vec4(0, 0, 0, 0) or vec4(1, 1, 1, 1).
    "	color =  light0(normal, globalAmbient) * u_LightSwitches[0];"
    "	color += light1(normal, globalAmbient) * u_LightSwitches[1];"
    "	color += light2(normal, globalAmbient) * u_LightSwitches[2];"
    "	color += light3(normal, globalAmbient) * u_LightSwitches[3];"
    "	color += light4(normal, globalAmbient) * u_LightSwitches[4];"
    "	color += light5(normal, globalAmbient) * u_LightSwitches[5];"
    "	color += light6(normal, globalAmbient) * u_LightSwitches[6];"
    "	color += light7(normal, globalAmbient) * u_LightSwitches[7];"
    "	return color;"
    "}"

    "void main()"
    "{"
    // Calculate and set light_color varying.
    "	v_LightColor = calculateLightColor();"

    "	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"

    "	gl_FrontColor = gl_Color;"

    "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
    "}";

static const char * const fShader_Texture01 =
    "#version 120 \n"

    "varying vec4 v_LightColor;"

    "uniform sampler2D u_SamplerIds[1];"
    "uniform vec2 u_Scales[1];"
    "uniform vec2 u_FlipFlags[1];"
    "uniform float u_TexAlphas[1];"

    "void main()"
    "{"
    "	vec4 resultColor;"

    // To avoid potential bugs, using uniform control flow as much as possible.

    // iBoundCoord is the Texture Coordinate within 0 and 1.
    "	vec2 iBoundCoord = gl_TexCoord[0].st;"

    // oBoundCoord is the Texture Coordinate outside 0 and 1 range.
    // Texture Map T Translate:
    // |----------| 2
    // |----------| 3
    // |----------| 4  <- 1.0
    // |          |
    // |          |
    // |----------| 0  <- (Scale)
    // |----------| 1  <- (Current TexCoord - Scale)
    "	vec2 oBoundCoord = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"

    // Flip S Coordinate if needed.
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord.s = 1 - iBoundCoord.s;"
    "		oBoundCoord.s = 1 - oBoundCoord.s;"
    "	}"

    // Flip T Coordinate if needed.
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord.t = 1 - iBoundCoord.t;"
    "		oBoundCoord.t = 1 - oBoundCoord.t;"
    "	}"

    // iBoundColor and oBoundColor are used to create smooth translation cross texture boundary.
    "	vec4 iBoundColor = texture2D(u_SamplerIds[0], iBoundCoord);"
    "	vec4 oBoundColor = texture2D(u_SamplerIds[0], oBoundCoord);"

    // Set Color base on the coordinate of texture.
    "	resultColor = iBoundColor;"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		resultColor = oBoundColor;"
    "	}"

    // Blend texture to geometry with appropriate alpha value.
    // Alpha value is calculate base on current location of texture. If texture
    // is on border(alpha equals 0), use alpha 0, else use alpha value from u_TexAlphas.
    "	resultColor = mix(gl_Color, resultColor, min(resultColor.a, u_TexAlphas[0]));"

    "	gl_FragColor = resultColor * v_LightColor;"
    "}";

/*!
* Shader Source Code for rendering two textures.
*
* vShader_MultiTexture02 calculates lighting color from up to eight light sources,
* and transforms textures' coordinate.
*
* fShader_MultiTexture02_Blending calculates blended texture color.
*
* fShader_MultiTexture02_Layering calculates layered texture color.
*/
static const char * const vShader_MultiTexture02 =
    "#version 120 \n"

    "uniform vec4 u_LightSwitches[8];"

    "varying vec4 v_LightColor;"

    "vec4 light0(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[0].position.xyz);"
    "	vec3 halfVector = gl_LightSource[0].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[0].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light1(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[1].position.xyz);"
    "	vec3 halfVector = gl_LightSource[1].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[1].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[1].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[1].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light2(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[2].position.xyz);"
    "	vec3 halfVector = gl_LightSource[2].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[2].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[2].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[2].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light3(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[3].position.xyz);"
    "	vec3 halfVector = gl_LightSource[3].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[3].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[3].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[3].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light4(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[4].position.xyz);"
    "	vec3 halfVector = gl_LightSource[4].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[4].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[4].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[4].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light5(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[5].position.xyz);"
    "	vec3 halfVector = gl_LightSource[5].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[5].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[5].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[5].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light6(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[6].position.xyz);"
    "	vec3 halfVector = gl_LightSource[6].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[6].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[6].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[6].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light7(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[7].position.xyz);"
    "	vec3 halfVector = gl_LightSource[7].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[7].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[7].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[7].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 calculateLightColor()"
    "{"
    "	vec4 color;"
    "	vec3 normal = normalize(gl_NormalMatrix * gl_Normal);"
    "	vec4 globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;"

    "	color =  light0(normal, globalAmbient) * u_LightSwitches[0];"
    "	color += light1(normal, globalAmbient) * u_LightSwitches[1];"
    "	color += light2(normal, globalAmbient) * u_LightSwitches[2];"
    "	color += light3(normal, globalAmbient) * u_LightSwitches[3];"
    "	color += light4(normal, globalAmbient) * u_LightSwitches[4];"
    "	color += light5(normal, globalAmbient) * u_LightSwitches[5];"
    "	color += light6(normal, globalAmbient) * u_LightSwitches[6];"
    "	color += light7(normal, globalAmbient) * u_LightSwitches[7];"
    "	return color;"
    "}"

    "void main()"
    "{"
    "	v_LightColor = calculateLightColor();"

    "	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
    "	gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord0;"

    "	gl_FrontColor = gl_Color;"

    "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
    "}";

static const char * const fShader_MultiTexture02_Blending =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[2];"
    "uniform vec2 u_Scales[2];"
    "uniform vec2 u_FlipFlags[2];"
    "uniform float u_TexAlphas[2];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    // inbound and outbound color for textures.
    "	vec4 iBoundColor[2];"
    "	vec4 oBoundColor[2];"

    // texture colors.
    "	vec4 texColor[2];"

    // inbound and outbound coordinate for textures.
    "	vec2 iBoundCoord[2];"
    "	vec2 oBoundCoord[2];"

    // Calculate Texture one's inbound and outbound coordinate.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"

    // Flip S Coordinate if needed.
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"

    //  Flip T Coordinate if needed.
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"

    // Calculate Texture one's inbound and outbound color.
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture two's inbound and outbound coordinate.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"

    // Flip S Coordinate if needed.
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"

    // Flip T Coordinate if needed.
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"

    // Calculate Texture two's inbound and outbound color.
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Check if current TexCoord is inbound or out.
    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"
    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    // Set Alphas.
    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"

    // Calculate blended color.
    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture02_Culling =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[2];"
    "uniform vec2 u_Scales[2];"
    "uniform vec2 u_FlipFlags[2];"
    "uniform float u_TexAlphas[2];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    // inbound and outbound color for textures.
    "	vec4 iBoundColor[2];"
    "	vec4 oBoundColor[2];"

    // texture colors.
    "	vec4 texColor[2];"

    // inbound and outbound coordinate for textures.
    "	vec2 iBoundCoord[2];"
    "	vec2 oBoundCoord[2];"

    // Calculate Texture one's inbound and outbound coordinate.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"

    // Flip S Coordinate if needed.
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"

    //  Flip T Coordinate if needed.
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"

    // Calculate Texture one's inbound and outbound color.
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture two's inbound and outbound coordinate.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"

    // Flip S Coordinate if needed.
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"

    // Flip T Coordinate if needed.
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"

    // Calculate Texture two's inbound and outbound color.
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Check if current TexCoord is inbound or out.
    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"
    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    // Set Alphas.
    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = min(texColor[1].a, u_TexAlphas[1]);"

    // Calculate blended color.
    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture02_Layering =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[2];"
    "uniform vec2 u_Scales[2];"
    "uniform vec2 u_FlipFlags[2];"
    "uniform float u_TexAlphas[2];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    // inbound and outbound color for textures.
    "	vec4 iBoundColor[2];"
    "	vec4 oBoundColor[2];"

    // texture colors.
    "	vec4 texColor[2];"

    // inbound and outbound coordinate for textures.
    "	vec2 iBoundCoord[2];"
    "	vec2 oBoundCoord[2];"

    // Calculate Texture one's inbound and outbound coordinate.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"

    // Flip S Coordinate if needed.
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"

    //  Flip T Coordinate if needed.
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"

    // Calculate Texture one's inbound and outbound color.
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture two's inbound and outbound coordinate.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"

    // Flip S Coordinate if needed.
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"

    // Flip T Coordinate if needed.
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"

    // Calculate Texture two's inbound and outbound color.
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Check if current TexCoord is inbound or out.
    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"
    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	vec4 layered_Color;"
    "  layered_Color = mix(gl_Color, texColor[0], min(texColor[0].a, u_TexAlphas[0]));"
    "	layered_Color = mix(layered_Color, texColor[1], min(texColor[1].a, u_TexAlphas[1]));"

    "	gl_FragColor = layered_Color * v_LightColor;"
    "}";

/*!
* Shader Source Code for rendering three textures.
*/
static const char * const vShader_MultiTexture03 =
    "#version 120 \n"

    "uniform vec4 u_LightSwitches[8];"

    "varying vec4 v_LightColor;"

    "vec4 light0(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[0].position.xyz);"
    "	vec3 halfVector = gl_LightSource[0].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[0].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light1(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[1].position.xyz);"
    "	vec3 halfVector = gl_LightSource[1].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[1].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[1].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[1].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light2(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[2].position.xyz);"
    "	vec3 halfVector = gl_LightSource[2].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[2].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[2].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[2].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light3(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[3].position.xyz);"
    "	vec3 halfVector = gl_LightSource[3].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[3].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[3].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[3].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light4(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[4].position.xyz);"
    "	vec3 halfVector = gl_LightSource[4].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[4].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[4].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[4].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light5(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[5].position.xyz);"
    "	vec3 halfVector = gl_LightSource[5].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[5].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[5].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[5].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light6(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[6].position.xyz);"
    "	vec3 halfVector = gl_LightSource[6].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[6].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[6].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[6].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light7(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[7].position.xyz);"
    "	vec3 halfVector = gl_LightSource[7].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[7].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[7].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[7].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 calculateLightColor()"
    "{"
    "	vec4 color;"
    "	vec3 normal = normalize(gl_NormalMatrix * gl_Normal);"
    "	vec4 globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;"

    //  Enable or disable light by multiply light color with vec4(0, 0, 0, 0) or vec4(1, 1, 1, 1).
    "	color =  light0(normal, globalAmbient) * u_LightSwitches[0];"
    "	color += light1(normal, globalAmbient) * u_LightSwitches[1];"
    "	color += light2(normal, globalAmbient) * u_LightSwitches[2];"
    "	color += light3(normal, globalAmbient) * u_LightSwitches[3];"
    "	color += light4(normal, globalAmbient) * u_LightSwitches[4];"
    "	color += light5(normal, globalAmbient) * u_LightSwitches[5];"
    "	color += light6(normal, globalAmbient) * u_LightSwitches[6];"
    "	color += light7(normal, globalAmbient) * u_LightSwitches[7];"
    "	return color;"
    "}"

    "void main()"
    "{"
    "	v_LightColor = calculateLightColor();"

    "	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
    "	gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord0;"
    "	gl_TexCoord[2] = gl_TextureMatrix[2] * gl_MultiTexCoord0;"

    "	gl_FrontColor = gl_Color;"

    "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
    "}";

static const char * const fShader_MultiTexture03_Blending =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[3];"
    "uniform vec2 u_Scales[3];"
    "uniform vec2 u_FlipFlags[3];"
    "uniform float u_TexAlphas[3];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[3];"
    "	vec4 oBoundColor[3];"
    "	vec2 iBoundCoord[3];"
    "	vec2 oBoundCoord[3];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Color.
    "	vec4 texColor[3];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"
    "	texColor[2].a = u_TexAlphas[2];"

    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1] * texColor[2];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture03_Culling =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[3];"
    "uniform vec2 u_Scales[3];"
    "uniform vec2 u_FlipFlags[3];"
    "uniform float u_TexAlphas[3];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[3];"
    "	vec4 oBoundColor[3];"
    "	vec2 iBoundCoord[3];"
    "	vec2 oBoundCoord[3];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Color.
    "	vec4 texColor[3];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"
    "	texColor[2].a = min(texColor[2].a, u_TexAlphas[2]);"

    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1] * texColor[2];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture03_Layering =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[3];"
    "uniform vec2 u_Scales[3];"
    "uniform vec2 u_FlipFlags[3];"
    "uniform float u_TexAlphas[3];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[3];"
    "	vec4 oBoundColor[3];"
    "	vec2 iBoundCoord[3];"
    "	vec2 oBoundCoord[3];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Color.
    "	vec4 texColor[3];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	vec4 layered_Color;"
    "	layered_Color = mix(gl_Color, texColor[0], min(texColor[0].a, u_TexAlphas[0]));"
    "	layered_Color = mix(layered_Color, texColor[1], min(texColor[1].a, u_TexAlphas[1]));"
    "	layered_Color = mix(layered_Color, texColor[2], min(texColor[2].a, u_TexAlphas[2]));"

    "	gl_FragColor = layered_Color * v_LightColor;"
    "}";

/*!
* Shader Source Code for rendering four textures.
*/
static const char * const vShader_MultiTexture04 =
    "#version 120 \n"

    "uniform vec4 u_LightSwitches[8];"

    "varying vec4 v_LightColor;"

    "vec4 light0(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[0].position.xyz);"
    "	vec3 halfVector = gl_LightSource[0].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[0].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light1(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[1].position.xyz);"
    "	vec3 halfVector = gl_LightSource[1].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[1].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[1].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[1].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light2(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[2].position.xyz);"
    "	vec3 halfVector = gl_LightSource[2].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[2].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[2].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[2].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light3(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[3].position.xyz);"
    "	vec3 halfVector = gl_LightSource[3].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[3].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[3].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[3].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light4(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[4].position.xyz);"
    "	vec3 halfVector = gl_LightSource[4].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[4].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[4].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[4].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light5(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[5].position.xyz);"
    "	vec3 halfVector = gl_LightSource[5].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[5].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[5].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[5].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light6(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[6].position.xyz);"
    "	vec3 halfVector = gl_LightSource[6].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[6].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[6].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[6].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light7(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[7].position.xyz);"
    "	vec3 halfVector = gl_LightSource[7].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[7].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[7].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[7].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 calculateLightColor()"
    "{"
    "	vec4 color;"
    "	vec3 normal = normalize(gl_NormalMatrix * gl_Normal);"
    "	vec4 globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;"

    //  Enable or disable light by multiply light color with vec4(0, 0, 0, 0) or vec4(1, 1, 1, 1).
    "	color =  light0(normal, globalAmbient) * u_LightSwitches[0];"
    "	color += light1(normal, globalAmbient) * u_LightSwitches[1];"
    "	color += light2(normal, globalAmbient) * u_LightSwitches[2];"
    "	color += light3(normal, globalAmbient) * u_LightSwitches[3];"
    "	color += light4(normal, globalAmbient) * u_LightSwitches[4];"
    "	color += light5(normal, globalAmbient) * u_LightSwitches[5];"
    "	color += light6(normal, globalAmbient) * u_LightSwitches[6];"
    "	color += light7(normal, globalAmbient) * u_LightSwitches[7];"
    "	return color;"
    "}"

    "void main()"
    "{"
    "	v_LightColor = calculateLightColor();"

    "	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
    "	gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord0;"
    "	gl_TexCoord[2] = gl_TextureMatrix[2] * gl_MultiTexCoord0;"
    "	gl_TexCoord[3] = gl_TextureMatrix[3] * gl_MultiTexCoord0;"

    "	gl_FrontColor = gl_Color;"

    "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
    "}";

static const char * const fShader_MultiTexture04_Blending =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[4];"
    "uniform vec2 u_Scales[4];"
    "uniform vec2 u_FlipFlags[4];"
    "uniform float u_TexAlphas[4];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[4];"
    "	vec4 oBoundColor[4];"
    "	vec2 iBoundCoord[4];"
    "	vec2 oBoundCoord[4];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Color.
    "	vec4 texColor[4];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"
    "	texColor[2].a = u_TexAlphas[2];"
    "	texColor[3].a = u_TexAlphas[3];"

    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1] * texColor[2] * texColor[3];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture04_Culling =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[4];"
    "uniform vec2 u_Scales[4];"
    "uniform vec2 u_FlipFlags[4];"
    "uniform float u_TexAlphas[4];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[4];"
    "	vec4 oBoundColor[4];"
    "	vec2 iBoundCoord[4];"
    "	vec2 oBoundCoord[4];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Color.
    "	vec4 texColor[4];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"
    "	texColor[2].a = u_TexAlphas[2];"
    "	texColor[3].a = min(texColor[3].a, u_TexAlphas[3]);"

    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1] * texColor[2] * texColor[3];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture04_Layering =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[4];"
    "uniform vec2 u_Scales[4];"
    "uniform vec2 u_FlipFlags[4];"
    "uniform float u_TexAlphas[4];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[4];"
    "	vec4 oBoundColor[4];"
    "	vec2 iBoundCoord[4];"
    "	vec2 oBoundCoord[4];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Color.
    "	vec4 texColor[4];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	vec4 layered_Color;"
    "	layered_Color = mix(gl_Color, texColor[0], min(texColor[0].a, u_TexAlphas[0]));"
    "	layered_Color = mix(layered_Color, texColor[1], min(texColor[1].a, u_TexAlphas[1]));"
    "	layered_Color = mix(layered_Color, texColor[2], min(texColor[2].a, u_TexAlphas[2]));"
    "	layered_Color = mix(layered_Color, texColor[3], min(texColor[3].a, u_TexAlphas[3]));"

    "	gl_FragColor = layered_Color * v_LightColor;"
    "}";

/*!
* Shader Source Code for rendering five textures.
*/
static const char * const vShader_MultiTexture05 =
    "#version 120 \n"

    "uniform vec4 u_LightSwitches[8];"

    "varying vec4 v_LightColor;"

    "vec4 light0(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[0].position.xyz);"
    "	vec3 halfVector = gl_LightSource[0].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[0].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light1(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[1].position.xyz);"
    "	vec3 halfVector = gl_LightSource[1].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[1].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[1].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[1].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light2(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[2].position.xyz);"
    "	vec3 halfVector = gl_LightSource[2].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[2].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[2].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[2].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light3(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[3].position.xyz);"
    "	vec3 halfVector = gl_LightSource[3].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[3].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[3].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[3].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light4(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[4].position.xyz);"
    "	vec3 halfVector = gl_LightSource[4].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[4].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[4].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[4].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light5(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[5].position.xyz);"
    "	vec3 halfVector = gl_LightSource[5].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[5].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[5].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[5].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light6(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[6].position.xyz);"
    "	vec3 halfVector = gl_LightSource[6].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[6].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[6].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[6].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light7(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[7].position.xyz);"
    "	vec3 halfVector = gl_LightSource[7].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[7].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[7].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[7].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 calculateLightColor()"
    "{"
    "	vec4 color;"
    "	vec3 normal = normalize(gl_NormalMatrix * gl_Normal);"
    "	vec4 globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;"

    //  Enable or disable light by multiply light color with vec4(0, 0, 0, 0) or vec4(1, 1, 1, 1).
    "	color =  light0(normal, globalAmbient) * u_LightSwitches[0];"
    "	color += light1(normal, globalAmbient) * u_LightSwitches[1];"
    "	color += light2(normal, globalAmbient) * u_LightSwitches[2];"
    "	color += light3(normal, globalAmbient) * u_LightSwitches[3];"
    "	color += light4(normal, globalAmbient) * u_LightSwitches[4];"
    "	color += light5(normal, globalAmbient) * u_LightSwitches[5];"
    "	color += light6(normal, globalAmbient) * u_LightSwitches[6];"
    "	color += light7(normal, globalAmbient) * u_LightSwitches[7];"
    "	return color;"
    "}"

    "void main()"
    "{"
    "	v_LightColor = calculateLightColor();"

    "	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
    "	gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord0;"
    "	gl_TexCoord[2] = gl_TextureMatrix[2] * gl_MultiTexCoord0;"
    "	gl_TexCoord[3] = gl_TextureMatrix[3] * gl_MultiTexCoord0;"
    "	gl_TexCoord[4] = gl_TextureMatrix[4] * gl_MultiTexCoord0;"

    "	gl_FrontColor = gl_Color;"

    "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
    "}";

static const char * const fShader_MultiTexture05_Blending =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[5];"
    "uniform vec2 u_Scales[5];"
    "uniform vec2 u_FlipFlags[5];"
    "uniform float u_TexAlphas[5];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[5];"
    "	vec4 oBoundColor[5];"
    "	vec2 iBoundCoord[5];"
    "	vec2 oBoundCoord[5];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Five's color.
    "	iBoundCoord[4] = gl_TexCoord[4].st;"
    "	oBoundCoord[4] = vec2(gl_TexCoord[4].s, gl_TexCoord[4].t - u_Scales[4].t);"
    "	if(u_FlipFlags[4].s == 1)"
    "	{"
    "		iBoundCoord[4].s = 1 - iBoundCoord[4].s;"
    "		oBoundCoord[4].s = 1 - oBoundCoord[4].s;"
    "	}"
    "	if(u_FlipFlags[4].t == 1)"
    "	{"
    "		iBoundCoord[4].t = 1 - iBoundCoord[4].t;"
    "		oBoundCoord[4].t = 1 - oBoundCoord[4].t;"
    "	}"
    "	iBoundColor[4] = texture2D(u_SamplerIds[4], iBoundCoord[4]);"
    "	oBoundColor[4] = texture2D(u_SamplerIds[4], oBoundCoord[4]);"

    // Calculate Texture Color.
    "	vec4 texColor[5];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[4] = iBoundColor[4];"
    "	if(gl_TexCoord[4].t > u_Scales[4].t)"
    "	{"
    "		texColor[4] = oBoundColor[4];"
    "	}"

    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"
    "	texColor[2].a = u_TexAlphas[2];"
    "	texColor[3].a = u_TexAlphas[3];"
    "	texColor[4].a = u_TexAlphas[4];"

    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1] * texColor[2] * texColor[3] * texColor[4];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture05_Culling =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[5];"
    "uniform vec2 u_Scales[5];"
    "uniform vec2 u_FlipFlags[5];"
    "uniform float u_TexAlphas[5];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[5];"
    "	vec4 oBoundColor[5];"
    "	vec2 iBoundCoord[5];"
    "	vec2 oBoundCoord[5];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Five's color.
    "	iBoundCoord[4] = gl_TexCoord[4].st;"
    "	oBoundCoord[4] = vec2(gl_TexCoord[4].s, gl_TexCoord[4].t - u_Scales[4].t);"
    "	if(u_FlipFlags[4].s == 1)"
    "	{"
    "		iBoundCoord[4].s = 1 - iBoundCoord[4].s;"
    "		oBoundCoord[4].s = 1 - oBoundCoord[4].s;"
    "	}"
    "	if(u_FlipFlags[4].t == 1)"
    "	{"
    "		iBoundCoord[4].t = 1 - iBoundCoord[4].t;"
    "		oBoundCoord[4].t = 1 - oBoundCoord[4].t;"
    "	}"
    "	iBoundColor[4] = texture2D(u_SamplerIds[4], iBoundCoord[4]);"
    "	oBoundColor[4] = texture2D(u_SamplerIds[4], oBoundCoord[4]);"

    // Calculate Texture Color.
    "	vec4 texColor[5];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[4] = iBoundColor[4];"
    "	if(gl_TexCoord[4].t > u_Scales[4].t)"
    "	{"
    "		texColor[4] = oBoundColor[4];"
    "	}"

    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"
    "	texColor[2].a = u_TexAlphas[2];"
    "	texColor[3].a = u_TexAlphas[3];"
    "	texColor[4].a = min(texColor[4].a, u_TexAlphas[4]);"

    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1] * texColor[2] * texColor[3] * texColor[4];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture05_Layering =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[5];"
    "uniform vec2 u_Scales[5];"
    "uniform vec2 u_FlipFlags[5];"
    "uniform float u_TexAlphas[5];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[5];"
    "	vec4 oBoundColor[5];"
    "	vec2 iBoundCoord[5];"
    "	vec2 oBoundCoord[5];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Five's color.
    "	iBoundCoord[4] = gl_TexCoord[4].st;"
    "	oBoundCoord[4] = vec2(gl_TexCoord[4].s, gl_TexCoord[4].t - u_Scales[4].t);"
    "	if(u_FlipFlags[4].s == 1)"
    "	{"
    "		iBoundCoord[4].s = 1 - iBoundCoord[4].s;"
    "		oBoundCoord[4].s = 1 - oBoundCoord[4].s;"
    "	}"
    "	if(u_FlipFlags[4].t == 1)"
    "	{"
    "		iBoundCoord[4].t = 1 - iBoundCoord[4].t;"
    "		oBoundCoord[4].t = 1 - oBoundCoord[4].t;"
    "	}"
    "	iBoundColor[4] = texture2D(u_SamplerIds[4], iBoundCoord[4]);"
    "	oBoundColor[4] = texture2D(u_SamplerIds[4], oBoundCoord[4]);"

    // Calculate Texture Color.
    "	vec4 texColor[5];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[4] = iBoundColor[4];"
    "	if(gl_TexCoord[4].t > u_Scales[4].t)"
    "	{"
    "		texColor[4] = oBoundColor[4];"
    "	}"

    "	vec4 layered_Color;"
    "	layered_Color = mix(gl_Color, texColor[0], min(texColor[0].a, u_TexAlphas[0]));"
    "	layered_Color = mix(layered_Color, texColor[1], min(texColor[1].a, u_TexAlphas[1]));"
    "	layered_Color = mix(layered_Color, texColor[2], min(texColor[2].a, u_TexAlphas[2]));"
    "	layered_Color = mix(layered_Color, texColor[3], min(texColor[3].a, u_TexAlphas[3]));"
    "	layered_Color = mix(layered_Color, texColor[4], min(texColor[4].a, u_TexAlphas[4]));"

    "	gl_FragColor = layered_Color * v_LightColor;"
    "}";

/*!
* Shader Source Code for rendering six textures.
*/
static const char * const vShader_MultiTexture06 =
    "#version 120 \n"

    "uniform vec4 u_LightSwitches[8];"

    "varying vec4 v_LightColor;"

    "vec4 light0(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[0].position.xyz);"
    "	vec3 halfVector = gl_LightSource[0].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[0].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light1(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[1].position.xyz);"
    "	vec3 halfVector = gl_LightSource[1].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[1].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[1].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[1].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light2(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[2].position.xyz);"
    "	vec3 halfVector = gl_LightSource[2].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[2].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[2].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[2].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light3(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[3].position.xyz);"
    "	vec3 halfVector = gl_LightSource[3].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[3].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[3].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[3].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light4(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[4].position.xyz);"
    "	vec3 halfVector = gl_LightSource[4].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[4].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[4].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[4].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light5(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[5].position.xyz);"
    "	vec3 halfVector = gl_LightSource[5].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[5].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[5].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[5].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light6(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[6].position.xyz);"
    "	vec3 halfVector = gl_LightSource[6].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[6].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[6].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[6].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light7(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[7].position.xyz);"
    "	vec3 halfVector = gl_LightSource[7].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[7].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[7].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[7].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 calculateLightColor()"
    "{"
    "	vec4 color;"
    "	vec3 normal = normalize(gl_NormalMatrix * gl_Normal);"
    "	vec4 globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;"

    //  Enable or disable light by multiply light color with vec4(0, 0, 0, 0) or vec4(1, 1, 1, 1).
    "	color =  light0(normal, globalAmbient) * u_LightSwitches[0];"
    "	color += light1(normal, globalAmbient) * u_LightSwitches[1];"
    "	color += light2(normal, globalAmbient) * u_LightSwitches[2];"
    "	color += light3(normal, globalAmbient) * u_LightSwitches[3];"
    "	color += light4(normal, globalAmbient) * u_LightSwitches[4];"
    "	color += light5(normal, globalAmbient) * u_LightSwitches[5];"
    "	color += light6(normal, globalAmbient) * u_LightSwitches[6];"
    "	color += light7(normal, globalAmbient) * u_LightSwitches[7];"
    "	return color;"
    "}"

    "void main()"
    "{"
    "	v_LightColor = calculateLightColor();"

    "	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
    "	gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord0;"
    "	gl_TexCoord[2] = gl_TextureMatrix[2] * gl_MultiTexCoord0;"
    "	gl_TexCoord[3] = gl_TextureMatrix[3] * gl_MultiTexCoord0;"
    "	gl_TexCoord[4] = gl_TextureMatrix[4] * gl_MultiTexCoord0;"
    "  gl_TexCoord[5] = gl_TextureMatrix[5] * gl_MultiTexCoord0;"

    "	gl_FrontColor = gl_Color;"

    "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
    "}";

static const char * const fShader_MultiTexture06_Blending =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[6];"
    "uniform vec2 u_Scales[6];"
    "uniform vec2 u_FlipFlags[6];"
    "uniform float u_TexAlphas[6];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[6];"
    "	vec4 oBoundColor[6];"
    "	vec2 iBoundCoord[6];"
    "	vec2 oBoundCoord[6];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Five's color.
    "	iBoundCoord[4] = gl_TexCoord[4].st;"
    "	oBoundCoord[4] = vec2(gl_TexCoord[4].s, gl_TexCoord[4].t - u_Scales[4].t);"
    "	if(u_FlipFlags[4].s == 1)"
    "	{"
    "		iBoundCoord[4].s = 1 - iBoundCoord[4].s;"
    "		oBoundCoord[4].s = 1 - oBoundCoord[4].s;"
    "	}"
    "	if(u_FlipFlags[4].t == 1)"
    "	{"
    "		iBoundCoord[4].t = 1 - iBoundCoord[4].t;"
    "		oBoundCoord[4].t = 1 - oBoundCoord[4].t;"
    "	}"
    "	iBoundColor[4] = texture2D(u_SamplerIds[4], iBoundCoord[4]);"
    "	oBoundColor[4] = texture2D(u_SamplerIds[4], oBoundCoord[4]);"

    // Calculate Texture Six's color.
    "	iBoundCoord[5] = gl_TexCoord[5].st;"
    "	oBoundCoord[5] = vec2(gl_TexCoord[5].s, gl_TexCoord[5].t - u_Scales[5].t);"
    "	if(u_FlipFlags[5].s == 1)"
    "	{"
    "		iBoundCoord[5].s = 1 - iBoundCoord[5].s;"
    "		oBoundCoord[5].s = 1 - oBoundCoord[5].s;"
    "	}"
    "	if(u_FlipFlags[5].t == 1)"
    "	{"
    "		iBoundCoord[5].t = 1 - iBoundCoord[5].t;"
    "		oBoundCoord[5].t = 1 - oBoundCoord[5].t;"
    "	}"
    "	iBoundColor[5] = texture2D(u_SamplerIds[5], iBoundCoord[5]);"
    "	oBoundColor[5] = texture2D(u_SamplerIds[5], oBoundCoord[5]);"

    // Calculate Texture Color.
    "	vec4 texColor[6];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[4] = iBoundColor[4];"
    "	if(gl_TexCoord[4].t > u_Scales[4].t)"
    "	{"
    "		texColor[4] = oBoundColor[4];"
    "	}"

    "	texColor[5] = iBoundColor[5];"
    "	if(gl_TexCoord[5].t > u_Scales[5].t)"
    "	{"
    "		texColor[5] = oBoundColor[5];"
    "	}"

    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"
    "	texColor[2].a = u_TexAlphas[2];"
    "	texColor[3].a = u_TexAlphas[3];"
    "	texColor[4].a = u_TexAlphas[4];"
    "	texColor[5].a = u_TexAlphas[5];"

    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1] * texColor[2] * texColor[3] * texColor[4] * texColor[5];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture06_Culling =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[6];"
    "uniform vec2 u_Scales[6];"
    "uniform vec2 u_FlipFlags[6];"
    "uniform float u_TexAlphas[6];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[6];"
    "	vec4 oBoundColor[6];"
    "	vec2 iBoundCoord[6];"
    "	vec2 oBoundCoord[6];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Five's color.
    "	iBoundCoord[4] = gl_TexCoord[4].st;"
    "	oBoundCoord[4] = vec2(gl_TexCoord[4].s, gl_TexCoord[4].t - u_Scales[4].t);"
    "	if(u_FlipFlags[4].s == 1)"
    "	{"
    "		iBoundCoord[4].s = 1 - iBoundCoord[4].s;"
    "		oBoundCoord[4].s = 1 - oBoundCoord[4].s;"
    "	}"
    "	if(u_FlipFlags[4].t == 1)"
    "	{"
    "		iBoundCoord[4].t = 1 - iBoundCoord[4].t;"
    "		oBoundCoord[4].t = 1 - oBoundCoord[4].t;"
    "	}"
    "	iBoundColor[4] = texture2D(u_SamplerIds[4], iBoundCoord[4]);"
    "	oBoundColor[4] = texture2D(u_SamplerIds[4], oBoundCoord[4]);"

    // Calculate Texture Six's color.
    "	iBoundCoord[5] = gl_TexCoord[5].st;"
    "	oBoundCoord[5] = vec2(gl_TexCoord[5].s, gl_TexCoord[5].t - u_Scales[5].t);"
    "	if(u_FlipFlags[5].s == 1)"
    "	{"
    "		iBoundCoord[5].s = 1 - iBoundCoord[5].s;"
    "		oBoundCoord[5].s = 1 - oBoundCoord[5].s;"
    "	}"
    "	if(u_FlipFlags[5].t == 1)"
    "	{"
    "		iBoundCoord[5].t = 1 - iBoundCoord[5].t;"
    "		oBoundCoord[5].t = 1 - oBoundCoord[5].t;"
    "	}"
    "	iBoundColor[5] = texture2D(u_SamplerIds[5], iBoundCoord[5]);"
    "	oBoundColor[5] = texture2D(u_SamplerIds[5], oBoundCoord[5]);"

    // Calculate Texture Color.
    "	vec4 texColor[6];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[4] = iBoundColor[4];"
    "	if(gl_TexCoord[4].t > u_Scales[4].t)"
    "	{"
    "		texColor[4] = oBoundColor[4];"
    "	}"

    "	texColor[5] = iBoundColor[5];"
    "	if(gl_TexCoord[5].t > u_Scales[5].t)"
    "	{"
    "		texColor[5] = oBoundColor[5];"
    "	}"

    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"
    "	texColor[2].a = u_TexAlphas[2];"
    "	texColor[3].a = u_TexAlphas[3];"
    "	texColor[4].a = u_TexAlphas[4];"
    "	texColor[5].a = min(texColor[5].a, u_TexAlphas[5]);"

    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1] * texColor[2] * texColor[3] * texColor[4] * texColor[5];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture06_Layering =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[6];"
    "uniform vec2 u_Scales[6];"
    "uniform vec2 u_FlipFlags[6];"
    "uniform float u_TexAlphas[6];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[6];"
    "	vec4 oBoundColor[6];"
    "	vec2 iBoundCoord[6];"
    "	vec2 oBoundCoord[6];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Five's color.
    "	iBoundCoord[4] = gl_TexCoord[4].st;"
    "	oBoundCoord[4] = vec2(gl_TexCoord[4].s, gl_TexCoord[4].t - u_Scales[4].t);"
    "	if(u_FlipFlags[4].s == 1)"
    "	{"
    "		iBoundCoord[4].s = 1 - iBoundCoord[4].s;"
    "		oBoundCoord[4].s = 1 - oBoundCoord[4].s;"
    "	}"
    "	if(u_FlipFlags[4].t == 1)"
    "	{"
    "		iBoundCoord[4].t = 1 - iBoundCoord[4].t;"
    "		oBoundCoord[4].t = 1 - oBoundCoord[4].t;"
    "	}"
    "	iBoundColor[4] = texture2D(u_SamplerIds[4], iBoundCoord[4]);"
    "	oBoundColor[4] = texture2D(u_SamplerIds[4], oBoundCoord[4]);"

    // Calculate Texture Six's color.
    "	iBoundCoord[5] = gl_TexCoord[5].st;"
    "	oBoundCoord[5] = vec2(gl_TexCoord[5].s, gl_TexCoord[5].t - u_Scales[5].t);"
    "	if(u_FlipFlags[5].s == 1)"
    "	{"
    "		iBoundCoord[5].s = 1 - iBoundCoord[5].s;"
    "		oBoundCoord[5].s = 1 - oBoundCoord[5].s;"
    "	}"
    "	if(u_FlipFlags[5].t == 1)"
    "	{"
    "		iBoundCoord[5].t = 1 - iBoundCoord[5].t;"
    "		oBoundCoord[5].t = 1 - oBoundCoord[5].t;"
    "	}"
    "	iBoundColor[5] = texture2D(u_SamplerIds[5], iBoundCoord[5]);"
    "	oBoundColor[5] = texture2D(u_SamplerIds[5], oBoundCoord[5]);"

    // Calculate Texture Color.
    "	vec4 texColor[6];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[4] = iBoundColor[4];"
    "	if(gl_TexCoord[4].t > u_Scales[4].t)"
    "	{"
    "		texColor[4] = oBoundColor[4];"
    "	}"

    "	texColor[5] = iBoundColor[5];"
    "	if(gl_TexCoord[5].t > u_Scales[5].t)"
    "	{"
    "		texColor[5] = oBoundColor[5];"
    "	}"

    "	vec4 layered_Color;"
    "	layered_Color = mix(gl_Color, texColor[0], min(texColor[0].a, u_TexAlphas[0]));"
    "	layered_Color = mix(layered_Color, texColor[1], min(texColor[1].a, u_TexAlphas[1]));"
    "	layered_Color = mix(layered_Color, texColor[2], min(texColor[2].a, u_TexAlphas[2]));"
    "	layered_Color = mix(layered_Color, texColor[3], min(texColor[3].a, u_TexAlphas[3]));"
    "	layered_Color = mix(layered_Color, texColor[4], min(texColor[4].a, u_TexAlphas[4]));"
    "	layered_Color = mix(layered_Color, texColor[5], min(texColor[5].a, u_TexAlphas[5]));"

    "	gl_FragColor = layered_Color * v_LightColor;"
    "}";

/*!
* Shader Source Code for rendering seven textures.
*/
static const char * const vShader_MultiTexture07 =
    "#version 120 \n"

    "uniform vec4 u_LightSwitches[8];"

    "varying vec4 v_LightColor;"

    "vec4 light0(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[0].position.xyz);"
    "	vec3 halfVector = gl_LightSource[0].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[0].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light1(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[1].position.xyz);"
    "	vec3 halfVector = gl_LightSource[1].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[1].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[1].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[1].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light2(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[2].position.xyz);"
    "	vec3 halfVector = gl_LightSource[2].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[2].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[2].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[2].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light3(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[3].position.xyz);"
    "	vec3 halfVector = gl_LightSource[3].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[3].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[3].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[3].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light4(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[4].position.xyz);"
    "	vec3 halfVector = gl_LightSource[4].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[4].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[4].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[4].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light5(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[5].position.xyz);"
    "	vec3 halfVector = gl_LightSource[5].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[5].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[5].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[5].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light6(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[6].position.xyz);"
    "	vec3 halfVector = gl_LightSource[6].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[6].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[6].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[6].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light7(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[7].position.xyz);"
    "	vec3 halfVector = gl_LightSource[7].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[7].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[7].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[7].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 calculateLightColor()"
    "{"
    "	vec4 color;"
    "	vec3 normal = normalize(gl_NormalMatrix * gl_Normal);"
    "	vec4 globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;"

    //  Enable or disable light by multiply light color with vec4(0, 0, 0, 0) or vec4(1, 1, 1, 1).
    "	color =  light0(normal, globalAmbient) * u_LightSwitches[0];"
    "	color += light1(normal, globalAmbient) * u_LightSwitches[1];"
    "	color += light2(normal, globalAmbient) * u_LightSwitches[2];"
    "	color += light3(normal, globalAmbient) * u_LightSwitches[3];"
    "	color += light4(normal, globalAmbient) * u_LightSwitches[4];"
    "	color += light5(normal, globalAmbient) * u_LightSwitches[5];"
    "	color += light6(normal, globalAmbient) * u_LightSwitches[6];"
    "	color += light7(normal, globalAmbient) * u_LightSwitches[7];"
    "	return color;"
    "}"

    "void main()"
    "{"
    "	v_LightColor = calculateLightColor();"

    "	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
    "	gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord0;"
    "	gl_TexCoord[2] = gl_TextureMatrix[2] * gl_MultiTexCoord0;"
    "	gl_TexCoord[3] = gl_TextureMatrix[3] * gl_MultiTexCoord0;"
    "	gl_TexCoord[4] = gl_TextureMatrix[4] * gl_MultiTexCoord0;"
    "  gl_TexCoord[5] = gl_TextureMatrix[5] * gl_MultiTexCoord0;"
    "  gl_TexCoord[6] = gl_TextureMatrix[6] * gl_MultiTexCoord0;"

    "	gl_FrontColor = gl_Color;"

    "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
    "}";

static const char * const fShader_MultiTexture07_Blending =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[7];"
    "uniform vec2 u_Scales[7];"
    "uniform vec2 u_FlipFlags[7];"
    "uniform float u_TexAlphas[7];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[7];"
    "	vec4 oBoundColor[7];"
    "	vec2 iBoundCoord[7];"
    "	vec2 oBoundCoord[7];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Five's color.
    "	iBoundCoord[4] = gl_TexCoord[4].st;"
    "	oBoundCoord[4] = vec2(gl_TexCoord[4].s, gl_TexCoord[4].t - u_Scales[4].t);"
    "	if(u_FlipFlags[4].s == 1)"
    "	{"
    "		iBoundCoord[4].s = 1 - iBoundCoord[4].s;"
    "		oBoundCoord[4].s = 1 - oBoundCoord[4].s;"
    "	}"
    "	if(u_FlipFlags[4].t == 1)"
    "	{"
    "		iBoundCoord[4].t = 1 - iBoundCoord[4].t;"
    "		oBoundCoord[4].t = 1 - oBoundCoord[4].t;"
    "	}"
    "	iBoundColor[4] = texture2D(u_SamplerIds[4], iBoundCoord[4]);"
    "	oBoundColor[4] = texture2D(u_SamplerIds[4], oBoundCoord[4]);"

    // Calculate Texture Six's color.
    "	iBoundCoord[5] = gl_TexCoord[5].st;"
    "	oBoundCoord[5] = vec2(gl_TexCoord[5].s, gl_TexCoord[5].t - u_Scales[5].t);"
    "	if(u_FlipFlags[5].s == 1)"
    "	{"
    "		iBoundCoord[5].s = 1 - iBoundCoord[5].s;"
    "		oBoundCoord[5].s = 1 - oBoundCoord[5].s;"
    "	}"
    "	if(u_FlipFlags[5].t == 1)"
    "	{"
    "		iBoundCoord[5].t = 1 - iBoundCoord[5].t;"
    "		oBoundCoord[5].t = 1 - oBoundCoord[5].t;"
    "	}"
    "	iBoundColor[5] = texture2D(u_SamplerIds[5], iBoundCoord[5]);"
    "	oBoundColor[5] = texture2D(u_SamplerIds[5], oBoundCoord[5]);"

    // Calculate Texture Seven's color.
    "	iBoundCoord[6] = gl_TexCoord[6].st;"
    "	oBoundCoord[6] = vec2(gl_TexCoord[6].s, gl_TexCoord[6].t - u_Scales[6].t);"
    "	if(u_FlipFlags[6].s == 1)"
    "	{"
    "		iBoundCoord[6].s = 1 - iBoundCoord[6].s;"
    "		oBoundCoord[6].s = 1 - oBoundCoord[6].s;"
    "	}"
    "	if(u_FlipFlags[6].t == 1)"
    "	{"
    "		iBoundCoord[6].t = 1 - iBoundCoord[6].t;"
    "		oBoundCoord[6].t = 1 - oBoundCoord[6].t;"
    "	}"
    "	iBoundColor[6] = texture2D(u_SamplerIds[6], iBoundCoord[6]);"
    "	oBoundColor[6] = texture2D(u_SamplerIds[6], oBoundCoord[6]);"

    // Calculate Texture Color.
    "	vec4 texColor[7];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[4] = iBoundColor[4];"
    "	if(gl_TexCoord[4].t > u_Scales[4].t)"
    "	{"
    "		texColor[4] = oBoundColor[4];"
    "	}"

    "	texColor[5] = iBoundColor[5];"
    "	if(gl_TexCoord[5].t > u_Scales[5].t)"
    "	{"
    "		texColor[5] = oBoundColor[5];"
    "	}"

    "	texColor[6] = iBoundColor[6];"
    "	if(gl_TexCoord[6].t > u_Scales[6].t)"
    "	{"
    "		texColor[6] = oBoundColor[6];"
    "	}"

    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"
    "	texColor[2].a = u_TexAlphas[2];"
    "	texColor[3].a = u_TexAlphas[3];"
    "	texColor[4].a = u_TexAlphas[4];"
    "	texColor[5].a = u_TexAlphas[5];"
    "	texColor[6].a = u_TexAlphas[6];"

    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1] * texColor[2] * texColor[3] * texColor[4] * texColor[5] * texColor[6];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture07_Culling =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[7];"
    "uniform vec2 u_Scales[7];"
    "uniform vec2 u_FlipFlags[7];"
    "uniform float u_TexAlphas[7];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[7];"
    "	vec4 oBoundColor[7];"
    "	vec2 iBoundCoord[7];"
    "	vec2 oBoundCoord[7];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Five's color.
    "	iBoundCoord[4] = gl_TexCoord[4].st;"
    "	oBoundCoord[4] = vec2(gl_TexCoord[4].s, gl_TexCoord[4].t - u_Scales[4].t);"
    "	if(u_FlipFlags[4].s == 1)"
    "	{"
    "		iBoundCoord[4].s = 1 - iBoundCoord[4].s;"
    "		oBoundCoord[4].s = 1 - oBoundCoord[4].s;"
    "	}"
    "	if(u_FlipFlags[4].t == 1)"
    "	{"
    "		iBoundCoord[4].t = 1 - iBoundCoord[4].t;"
    "		oBoundCoord[4].t = 1 - oBoundCoord[4].t;"
    "	}"
    "	iBoundColor[4] = texture2D(u_SamplerIds[4], iBoundCoord[4]);"
    "	oBoundColor[4] = texture2D(u_SamplerIds[4], oBoundCoord[4]);"

    // Calculate Texture Six's color.
    "	iBoundCoord[5] = gl_TexCoord[5].st;"
    "	oBoundCoord[5] = vec2(gl_TexCoord[5].s, gl_TexCoord[5].t - u_Scales[5].t);"
    "	if(u_FlipFlags[5].s == 1)"
    "	{"
    "		iBoundCoord[5].s = 1 - iBoundCoord[5].s;"
    "		oBoundCoord[5].s = 1 - oBoundCoord[5].s;"
    "	}"
    "	if(u_FlipFlags[5].t == 1)"
    "	{"
    "		iBoundCoord[5].t = 1 - iBoundCoord[5].t;"
    "		oBoundCoord[5].t = 1 - oBoundCoord[5].t;"
    "	}"
    "	iBoundColor[5] = texture2D(u_SamplerIds[5], iBoundCoord[5]);"
    "	oBoundColor[5] = texture2D(u_SamplerIds[5], oBoundCoord[5]);"

    // Calculate Texture Seven's color.
    "	iBoundCoord[6] = gl_TexCoord[6].st;"
    "	oBoundCoord[6] = vec2(gl_TexCoord[6].s, gl_TexCoord[6].t - u_Scales[6].t);"
    "	if(u_FlipFlags[6].s == 1)"
    "	{"
    "		iBoundCoord[6].s = 1 - iBoundCoord[6].s;"
    "		oBoundCoord[6].s = 1 - oBoundCoord[6].s;"
    "	}"
    "	if(u_FlipFlags[6].t == 1)"
    "	{"
    "		iBoundCoord[6].t = 1 - iBoundCoord[6].t;"
    "		oBoundCoord[6].t = 1 - oBoundCoord[6].t;"
    "	}"
    "	iBoundColor[6] = texture2D(u_SamplerIds[6], iBoundCoord[6]);"
    "	oBoundColor[6] = texture2D(u_SamplerIds[6], oBoundCoord[6]);"

    // Calculate Texture Color.
    "	vec4 texColor[7];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[4] = iBoundColor[4];"
    "	if(gl_TexCoord[4].t > u_Scales[4].t)"
    "	{"
    "		texColor[4] = oBoundColor[4];"
    "	}"

    "	texColor[5] = iBoundColor[5];"
    "	if(gl_TexCoord[5].t > u_Scales[5].t)"
    "	{"
    "		texColor[5] = oBoundColor[5];"
    "	}"

    "	texColor[6] = iBoundColor[6];"
    "	if(gl_TexCoord[6].t > u_Scales[6].t)"
    "	{"
    "		texColor[6] = oBoundColor[6];"
    "	}"

    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"
    "	texColor[2].a = u_TexAlphas[2];"
    "	texColor[3].a = u_TexAlphas[3];"
    "	texColor[4].a = u_TexAlphas[4];"
    "	texColor[5].a = u_TexAlphas[5];"
    "	texColor[6].a = min(texColor[6].a, u_TexAlphas[6]);"

    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1] * texColor[2] * texColor[3] * texColor[4] * texColor[5] * texColor[6];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture07_Layering =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[7];"
    "uniform vec2 u_Scales[7];"
    "uniform vec2 u_FlipFlags[7];"
    "uniform float u_TexAlphas[7];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[7];"
    "	vec4 oBoundColor[7];"
    "	vec2 iBoundCoord[7];"
    "	vec2 oBoundCoord[7];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Five's color.
    "	iBoundCoord[4] = gl_TexCoord[4].st;"
    "	oBoundCoord[4] = vec2(gl_TexCoord[4].s, gl_TexCoord[4].t - u_Scales[4].t);"
    "	if(u_FlipFlags[4].s == 1)"
    "	{"
    "		iBoundCoord[4].s = 1 - iBoundCoord[4].s;"
    "		oBoundCoord[4].s = 1 - oBoundCoord[4].s;"
    "	}"
    "	if(u_FlipFlags[4].t == 1)"
    "	{"
    "		iBoundCoord[4].t = 1 - iBoundCoord[4].t;"
    "		oBoundCoord[4].t = 1 - oBoundCoord[4].t;"
    "	}"
    "	iBoundColor[4] = texture2D(u_SamplerIds[4], iBoundCoord[4]);"
    "	oBoundColor[4] = texture2D(u_SamplerIds[4], oBoundCoord[4]);"

    // Calculate Texture Six's color.
    "	iBoundCoord[5] = gl_TexCoord[5].st;"
    "	oBoundCoord[5] = vec2(gl_TexCoord[5].s, gl_TexCoord[5].t - u_Scales[5].t);"
    "	if(u_FlipFlags[5].s == 1)"
    "	{"
    "		iBoundCoord[5].s = 1 - iBoundCoord[5].s;"
    "		oBoundCoord[5].s = 1 - oBoundCoord[5].s;"
    "	}"
    "	if(u_FlipFlags[5].t == 1)"
    "	{"
    "		iBoundCoord[5].t = 1 - iBoundCoord[5].t;"
    "		oBoundCoord[5].t = 1 - oBoundCoord[5].t;"
    "	}"
    "	iBoundColor[5] = texture2D(u_SamplerIds[5], iBoundCoord[5]);"
    "	oBoundColor[5] = texture2D(u_SamplerIds[5], oBoundCoord[5]);"

    // Calculate Texture Seven's color.
    "	iBoundCoord[6] = gl_TexCoord[6].st;"
    "	oBoundCoord[6] = vec2(gl_TexCoord[6].s, gl_TexCoord[6].t - u_Scales[6].t);"
    "	if(u_FlipFlags[6].s == 1)"
    "	{"
    "		iBoundCoord[6].s = 1 - iBoundCoord[6].s;"
    "		oBoundCoord[6].s = 1 - oBoundCoord[6].s;"
    "	}"
    "	if(u_FlipFlags[6].t == 1)"
    "	{"
    "		iBoundCoord[6].t = 1 - iBoundCoord[6].t;"
    "		oBoundCoord[6].t = 1 - oBoundCoord[6].t;"
    "	}"
    "	iBoundColor[6] = texture2D(u_SamplerIds[6], iBoundCoord[6]);"
    "	oBoundColor[6] = texture2D(u_SamplerIds[6], oBoundCoord[6]);"

    // Calculate Texture Color.
    "	vec4 texColor[7];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[4] = iBoundColor[4];"
    "	if(gl_TexCoord[4].t > u_Scales[4].t)"
    "	{"
    "		texColor[4] = oBoundColor[4];"
    "	}"

    "	texColor[5] = iBoundColor[5];"
    "	if(gl_TexCoord[5].t > u_Scales[5].t)"
    "	{"
    "		texColor[5] = oBoundColor[5];"
    "	}"

    "	texColor[6] = iBoundColor[6];"
    "	if(gl_TexCoord[6].t > u_Scales[6].t)"
    "	{"
    "		texColor[6] = oBoundColor[6];"
    "	}"

    "	vec4 layered_Color;"
    "	layered_Color = mix(gl_Color, texColor[0], min(texColor[0].a, u_TexAlphas[0]));"
    "	layered_Color = mix(layered_Color, texColor[1], min(texColor[1].a, u_TexAlphas[1]));"
    "	layered_Color = mix(layered_Color, texColor[2], min(texColor[2].a, u_TexAlphas[2]));"
    "	layered_Color = mix(layered_Color, texColor[3], min(texColor[3].a, u_TexAlphas[3]));"
    "	layered_Color = mix(layered_Color, texColor[4], min(texColor[4].a, u_TexAlphas[4]));"
    "	layered_Color = mix(layered_Color, texColor[5], min(texColor[5].a, u_TexAlphas[5]));"
    "	layered_Color = mix(layered_Color, texColor[6], min(texColor[6].a, u_TexAlphas[6]));"

    "	gl_FragColor = layered_Color * v_LightColor;"
    "}";

/*!
* Shader Source Code for rendering eight textures.
*/
static const char * const vShader_MultiTexture08 =
    "#version 120 \n"

    "uniform vec4 u_LightSwitches[8];"

    "varying vec4 v_LightColor;"

    "vec4 light0(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[0].position.xyz);"
    "	vec3 halfVector = gl_LightSource[0].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[0].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light1(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[1].position.xyz);"
    "	vec3 halfVector = gl_LightSource[1].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[1].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[1].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[1].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light2(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[2].position.xyz);"
    "	vec3 halfVector = gl_LightSource[2].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[2].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[2].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[2].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light3(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[3].position.xyz);"
    "	vec3 halfVector = gl_LightSource[3].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[3].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[3].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[3].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light4(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[4].position.xyz);"
    "	vec3 halfVector = gl_LightSource[4].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[4].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[4].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[4].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light5(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[5].position.xyz);"
    "	vec3 halfVector = gl_LightSource[5].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[5].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[5].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[5].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light6(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[6].position.xyz);"
    "	vec3 halfVector = gl_LightSource[6].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[6].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[6].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[6].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 light7(vec3 normal, vec4 globalAmbient)"
    "{"
    "	vec3 lightDir = normalize(gl_LightSource[7].position.xyz);"
    "	vec3 halfVector = gl_LightSource[7].halfVector.xyz;"

    "	vec4 ambient = gl_LightSource[7].ambient * gl_FrontMaterial.ambient + globalAmbient;"
    "	vec4 diffuse = gl_LightSource[7].diffuse * gl_FrontMaterial.diffuse * max(dot(normal, lightDir), 0.0);"
    "	vec4 specular = gl_LightSource[7].specular * gl_FrontMaterial.specular * pow(max(dot(normal, halfVector), 0.0), gl_FrontMaterial.shininess);"
    "	return ambient + diffuse + specular;"
    "}"

    "vec4 calculateLightColor()"
    "{"
    "	vec4 color;"
    "	vec3 normal = normalize(gl_NormalMatrix * gl_Normal);"
    "	vec4 globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;"

    //  Enable or disable light by multiply light color with vec4(0, 0, 0, 0) or vec4(1, 1, 1, 1).
    "	color =  light0(normal, globalAmbient) * u_LightSwitches[0];"
    "	color += light1(normal, globalAmbient) * u_LightSwitches[1];"
    "	color += light2(normal, globalAmbient) * u_LightSwitches[2];"
    "	color += light3(normal, globalAmbient) * u_LightSwitches[3];"
    "	color += light4(normal, globalAmbient) * u_LightSwitches[4];"
    "	color += light5(normal, globalAmbient) * u_LightSwitches[5];"
    "	color += light6(normal, globalAmbient) * u_LightSwitches[6];"
    "	color += light7(normal, globalAmbient) * u_LightSwitches[7];"
    "	return color;"
    "}"

    "void main()"
    "{"
    "	v_LightColor = calculateLightColor();"

    "	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
    "	gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord0;"
    "	gl_TexCoord[2] = gl_TextureMatrix[2] * gl_MultiTexCoord0;"
    "	gl_TexCoord[3] = gl_TextureMatrix[3] * gl_MultiTexCoord0;"
    "	gl_TexCoord[4] = gl_TextureMatrix[4] * gl_MultiTexCoord0;"
    "  gl_TexCoord[5] = gl_TextureMatrix[5] * gl_MultiTexCoord0;"
    "  gl_TexCoord[6] = gl_TextureMatrix[6] * gl_MultiTexCoord0;"
    "  gl_TexCoord[7] = gl_TextureMatrix[7] * gl_MultiTexCoord0;"

    "	gl_FrontColor = gl_Color;"

    "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
    "}";

static const char * const fShader_MultiTexture08_Blending =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[8];"
    "uniform vec2 u_Scales[8];"
    "uniform vec2 u_FlipFlags[8];"
    "uniform float u_TexAlphas[8];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[8];"
    "	vec4 oBoundColor[8];"
    "	vec2 iBoundCoord[8];"
    "	vec2 oBoundCoord[8];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Five's color.
    "	iBoundCoord[4] = gl_TexCoord[4].st;"
    "	oBoundCoord[4] = vec2(gl_TexCoord[4].s, gl_TexCoord[4].t - u_Scales[4].t);"
    "	if(u_FlipFlags[4].s == 1)"
    "	{"
    "		iBoundCoord[4].s = 1 - iBoundCoord[4].s;"
    "		oBoundCoord[4].s = 1 - oBoundCoord[4].s;"
    "	}"
    "	if(u_FlipFlags[4].t == 1)"
    "	{"
    "		iBoundCoord[4].t = 1 - iBoundCoord[4].t;"
    "		oBoundCoord[4].t = 1 - oBoundCoord[4].t;"
    "	}"
    "	iBoundColor[4] = texture2D(u_SamplerIds[4], iBoundCoord[4]);"
    "	oBoundColor[4] = texture2D(u_SamplerIds[4], oBoundCoord[4]);"

    // Calculate Texture Six's color.
    "	iBoundCoord[5] = gl_TexCoord[5].st;"
    "	oBoundCoord[5] = vec2(gl_TexCoord[5].s, gl_TexCoord[5].t - u_Scales[5].t);"
    "	if(u_FlipFlags[5].s == 1)"
    "	{"
    "		iBoundCoord[5].s = 1 - iBoundCoord[5].s;"
    "		oBoundCoord[5].s = 1 - oBoundCoord[5].s;"
    "	}"
    "	if(u_FlipFlags[5].t == 1)"
    "	{"
    "		iBoundCoord[5].t = 1 - iBoundCoord[5].t;"
    "		oBoundCoord[5].t = 1 - oBoundCoord[5].t;"
    "	}"
    "	iBoundColor[5] = texture2D(u_SamplerIds[5], iBoundCoord[5]);"
    "	oBoundColor[5] = texture2D(u_SamplerIds[5], oBoundCoord[5]);"

    // Calculate Texture Seven's color.
    "	iBoundCoord[6] = gl_TexCoord[6].st;"
    "	oBoundCoord[6] = vec2(gl_TexCoord[6].s, gl_TexCoord[6].t - u_Scales[6].t);"
    "	if(u_FlipFlags[6].s == 1)"
    "	{"
    "		iBoundCoord[6].s = 1 - iBoundCoord[6].s;"
    "		oBoundCoord[6].s = 1 - oBoundCoord[6].s;"
    "	}"
    "	if(u_FlipFlags[6].t == 1)"
    "	{"
    "		iBoundCoord[6].t = 1 - iBoundCoord[6].t;"
    "		oBoundCoord[6].t = 1 - oBoundCoord[6].t;"
    "	}"
    "	iBoundColor[6] = texture2D(u_SamplerIds[6], iBoundCoord[6]);"
    "	oBoundColor[6] = texture2D(u_SamplerIds[6], oBoundCoord[6]);"

    // Calculate Texture Eight's color.
    "	iBoundCoord[7] = gl_TexCoord[7].st;"
    "	oBoundCoord[7] = vec2(gl_TexCoord[7].s, gl_TexCoord[7].t - u_Scales[7].t);"
    "	if(u_FlipFlags[7].s == 1)"
    "	{"
    "		iBoundCoord[7].s = 1 - iBoundCoord[7].s;"
    "		oBoundCoord[7].s = 1 - oBoundCoord[7].s;"
    "	}"
    "	if(u_FlipFlags[7].t == 1)"
    "	{"
    "		iBoundCoord[7].t = 1 - iBoundCoord[7].t;"
    "		oBoundCoord[7].t = 1 - oBoundCoord[7].t;"
    "	}"
    "	iBoundColor[7] = texture2D(u_SamplerIds[7], iBoundCoord[7]);"
    "	oBoundColor[7] = texture2D(u_SamplerIds[7], oBoundCoord[7]);"

    // Calculate Texture Color.
    "	vec4 texColor[8];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[4] = iBoundColor[4];"
    "	if(gl_TexCoord[4].t > u_Scales[4].t)"
    "	{"
    "		texColor[4] = oBoundColor[4];"
    "	}"

    "	texColor[5] = iBoundColor[5];"
    "	if(gl_TexCoord[5].t > u_Scales[5].t)"
    "	{"
    "		texColor[5] = oBoundColor[5];"
    "	}"

    "	texColor[6] = iBoundColor[6];"
    "	if(gl_TexCoord[6].t > u_Scales[6].t)"
    "	{"
    "		texColor[6] = oBoundColor[6];"
    "	}"

    "	texColor[7] = iBoundColor[7];"
    "	if(gl_TexCoord[7].t > u_Scales[7].t)"
    "	{"
    "		texColor[7] = oBoundColor[7];"
    "	}"

    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"
    "	texColor[2].a = u_TexAlphas[2];"
    "	texColor[3].a = u_TexAlphas[3];"
    "	texColor[4].a = u_TexAlphas[4];"
    "	texColor[5].a = u_TexAlphas[5];"
    "	texColor[6].a = u_TexAlphas[6];"
    "	texColor[7].a = u_TexAlphas[7];"

    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1] * texColor[2] * texColor[3] * texColor[4] * texColor[5] * texColor[6] * texColor[7];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture08_Culling =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[8];"
    "uniform vec2 u_Scales[8];"
    "uniform vec2 u_FlipFlags[8];"
    "uniform float u_TexAlphas[8];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[8];"
    "	vec4 oBoundColor[8];"
    "	vec2 iBoundCoord[8];"
    "	vec2 oBoundCoord[8];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Five's color.
    "	iBoundCoord[4] = gl_TexCoord[4].st;"
    "	oBoundCoord[4] = vec2(gl_TexCoord[4].s, gl_TexCoord[4].t - u_Scales[4].t);"
    "	if(u_FlipFlags[4].s == 1)"
    "	{"
    "		iBoundCoord[4].s = 1 - iBoundCoord[4].s;"
    "		oBoundCoord[4].s = 1 - oBoundCoord[4].s;"
    "	}"
    "	if(u_FlipFlags[4].t == 1)"
    "	{"
    "		iBoundCoord[4].t = 1 - iBoundCoord[4].t;"
    "		oBoundCoord[4].t = 1 - oBoundCoord[4].t;"
    "	}"
    "	iBoundColor[4] = texture2D(u_SamplerIds[4], iBoundCoord[4]);"
    "	oBoundColor[4] = texture2D(u_SamplerIds[4], oBoundCoord[4]);"

    // Calculate Texture Six's color.
    "	iBoundCoord[5] = gl_TexCoord[5].st;"
    "	oBoundCoord[5] = vec2(gl_TexCoord[5].s, gl_TexCoord[5].t - u_Scales[5].t);"
    "	if(u_FlipFlags[5].s == 1)"
    "	{"
    "		iBoundCoord[5].s = 1 - iBoundCoord[5].s;"
    "		oBoundCoord[5].s = 1 - oBoundCoord[5].s;"
    "	}"
    "	if(u_FlipFlags[5].t == 1)"
    "	{"
    "		iBoundCoord[5].t = 1 - iBoundCoord[5].t;"
    "		oBoundCoord[5].t = 1 - oBoundCoord[5].t;"
    "	}"
    "	iBoundColor[5] = texture2D(u_SamplerIds[5], iBoundCoord[5]);"
    "	oBoundColor[5] = texture2D(u_SamplerIds[5], oBoundCoord[5]);"

    // Calculate Texture Seven's color.
    "	iBoundCoord[6] = gl_TexCoord[6].st;"
    "	oBoundCoord[6] = vec2(gl_TexCoord[6].s, gl_TexCoord[6].t - u_Scales[6].t);"
    "	if(u_FlipFlags[6].s == 1)"
    "	{"
    "		iBoundCoord[6].s = 1 - iBoundCoord[6].s;"
    "		oBoundCoord[6].s = 1 - oBoundCoord[6].s;"
    "	}"
    "	if(u_FlipFlags[6].t == 1)"
    "	{"
    "		iBoundCoord[6].t = 1 - iBoundCoord[6].t;"
    "		oBoundCoord[6].t = 1 - oBoundCoord[6].t;"
    "	}"
    "	iBoundColor[6] = texture2D(u_SamplerIds[6], iBoundCoord[6]);"
    "	oBoundColor[6] = texture2D(u_SamplerIds[6], oBoundCoord[6]);"

    // Calculate Texture Eight's color.
    "	iBoundCoord[7] = gl_TexCoord[7].st;"
    "	oBoundCoord[7] = vec2(gl_TexCoord[7].s, gl_TexCoord[7].t - u_Scales[7].t);"
    "	if(u_FlipFlags[7].s == 1)"
    "	{"
    "		iBoundCoord[7].s = 1 - iBoundCoord[7].s;"
    "		oBoundCoord[7].s = 1 - oBoundCoord[7].s;"
    "	}"
    "	if(u_FlipFlags[7].t == 1)"
    "	{"
    "		iBoundCoord[7].t = 1 - iBoundCoord[7].t;"
    "		oBoundCoord[7].t = 1 - oBoundCoord[7].t;"
    "	}"
    "	iBoundColor[7] = texture2D(u_SamplerIds[7], iBoundCoord[7]);"
    "	oBoundColor[7] = texture2D(u_SamplerIds[7], oBoundCoord[7]);"

    // Calculate Texture Color.
    "	vec4 texColor[8];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[4] = iBoundColor[4];"
    "	if(gl_TexCoord[4].t > u_Scales[4].t)"
    "	{"
    "		texColor[4] = oBoundColor[4];"
    "	}"

    "	texColor[5] = iBoundColor[5];"
    "	if(gl_TexCoord[5].t > u_Scales[5].t)"
    "	{"
    "		texColor[5] = oBoundColor[5];"
    "	}"

    "	texColor[6] = iBoundColor[6];"
    "	if(gl_TexCoord[6].t > u_Scales[6].t)"
    "	{"
    "		texColor[6] = oBoundColor[6];"
    "	}"

    "	texColor[7] = iBoundColor[7];"
    "	if(gl_TexCoord[7].t > u_Scales[7].t)"
    "	{"
    "		texColor[7] = oBoundColor[7];"
    "	}"

    "	texColor[0].a = u_TexAlphas[0];"
    "	texColor[1].a = u_TexAlphas[1];"
    "	texColor[2].a = u_TexAlphas[2];"
    "	texColor[3].a = u_TexAlphas[3];"
    "	texColor[4].a = u_TexAlphas[4];"
    "	texColor[5].a = u_TexAlphas[5];"
    "	texColor[6].a = u_TexAlphas[6];"
    "	texColor[7].a = min(texColor[7].a, u_TexAlphas[7]);"

    "	vec4 blended_Color;"
    "	blended_Color = texColor[0] * texColor[1] * texColor[2] * texColor[3] * texColor[4] * texColor[5] * texColor[6] * texColor[7];"
    "	blended_Color = mix(gl_Color, blended_Color, blended_Color.a);"

    "	gl_FragColor = blended_Color * v_LightColor;"
    "}";

static const char * const fShader_MultiTexture08_Layering =
    "#version 120 \n"

    "uniform sampler2D u_SamplerIds[8];"
    "uniform vec2 u_Scales[8];"
    "uniform vec2 u_FlipFlags[8];"
    "uniform float u_TexAlphas[8];"

    "varying vec4 v_LightColor;"

    "void main()"
    "{"
    "	vec4 iBoundColor[8];"
    "	vec4 oBoundColor[8];"
    "	vec2 iBoundCoord[8];"
    "	vec2 oBoundCoord[8];"

    // Calculate Texture One's color.
    "	iBoundCoord[0] = gl_TexCoord[0].st;"
    "	oBoundCoord[0] = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - u_Scales[0].t);"
    "	if(u_FlipFlags[0].s == 1)"
    "	{"
    "		iBoundCoord[0].s = 1 - iBoundCoord[0].s;"
    "		oBoundCoord[0].s = 1 - oBoundCoord[0].s;"
    "	}"
    "	if(u_FlipFlags[0].t == 1)"
    "	{"
    "		iBoundCoord[0].t = 1 - iBoundCoord[0].t;"
    "		oBoundCoord[0].t = 1 - oBoundCoord[0].t;"
    "	}"
    "	iBoundColor[0] = texture2D(u_SamplerIds[0], iBoundCoord[0]);"
    "	oBoundColor[0] = texture2D(u_SamplerIds[0], oBoundCoord[0]);"

    // Calculate Texture Two's color.
    "	iBoundCoord[1] = gl_TexCoord[1].st;"
    "	oBoundCoord[1] = vec2(gl_TexCoord[1].s, gl_TexCoord[1].t - u_Scales[1].t);"
    "	if(u_FlipFlags[1].s == 1)"
    "	{"
    "		iBoundCoord[1].s = 1 - iBoundCoord[1].s;"
    "		oBoundCoord[1].s = 1 - oBoundCoord[1].s;"
    "	}"
    "	if(u_FlipFlags[1].t == 1)"
    "	{"
    "		iBoundCoord[1].t = 1 - iBoundCoord[1].t;"
    "		oBoundCoord[1].t = 1 - oBoundCoord[1].t;"
    "	}"
    "	iBoundColor[1] = texture2D(u_SamplerIds[1], iBoundCoord[1]);"
    "	oBoundColor[1] = texture2D(u_SamplerIds[1], oBoundCoord[1]);"

    // Calculate Texture Three's color.
    "	iBoundCoord[2] = gl_TexCoord[2].st;"
    "	oBoundCoord[2] = vec2(gl_TexCoord[2].s, gl_TexCoord[2].t - u_Scales[2].t);"
    "	if(u_FlipFlags[2].s == 1)"
    "	{"
    "		iBoundCoord[2].s = 1 - iBoundCoord[2].s;"
    "		oBoundCoord[2].s = 1 - oBoundCoord[2].s;"
    "	}"
    "	if(u_FlipFlags[2].t == 1)"
    "	{"
    "		iBoundCoord[2].t = 1 - iBoundCoord[2].t;"
    "		oBoundCoord[2].t = 1 - oBoundCoord[2].t;"
    "	}"
    "	iBoundColor[2] = texture2D(u_SamplerIds[2], iBoundCoord[2]);"
    "	oBoundColor[2] = texture2D(u_SamplerIds[2], oBoundCoord[2]);"

    // Calculate Texture Four's color.
    "	iBoundCoord[3] = gl_TexCoord[3].st;"
    "	oBoundCoord[3] = vec2(gl_TexCoord[3].s, gl_TexCoord[3].t - u_Scales[3].t);"
    "	if(u_FlipFlags[3].s == 1)"
    "	{"
    "		iBoundCoord[3].s = 1 - iBoundCoord[3].s;"
    "		oBoundCoord[3].s = 1 - oBoundCoord[3].s;"
    "	}"
    "	if(u_FlipFlags[3].t == 1)"
    "	{"
    "		iBoundCoord[3].t = 1 - iBoundCoord[3].t;"
    "		oBoundCoord[3].t = 1 - oBoundCoord[3].t;"
    "	}"
    "	iBoundColor[3] = texture2D(u_SamplerIds[3], iBoundCoord[3]);"
    "	oBoundColor[3] = texture2D(u_SamplerIds[3], oBoundCoord[3]);"

    // Calculate Texture Five's color.
    "	iBoundCoord[4] = gl_TexCoord[4].st;"
    "	oBoundCoord[4] = vec2(gl_TexCoord[4].s, gl_TexCoord[4].t - u_Scales[4].t);"
    "	if(u_FlipFlags[4].s == 1)"
    "	{"
    "		iBoundCoord[4].s = 1 - iBoundCoord[4].s;"
    "		oBoundCoord[4].s = 1 - oBoundCoord[4].s;"
    "	}"
    "	if(u_FlipFlags[4].t == 1)"
    "	{"
    "		iBoundCoord[4].t = 1 - iBoundCoord[4].t;"
    "		oBoundCoord[4].t = 1 - oBoundCoord[4].t;"
    "	}"
    "	iBoundColor[4] = texture2D(u_SamplerIds[4], iBoundCoord[4]);"
    "	oBoundColor[4] = texture2D(u_SamplerIds[4], oBoundCoord[4]);"

    // Calculate Texture Six's color.
    "	iBoundCoord[5] = gl_TexCoord[5].st;"
    "	oBoundCoord[5] = vec2(gl_TexCoord[5].s, gl_TexCoord[5].t - u_Scales[5].t);"
    "	if(u_FlipFlags[5].s == 1)"
    "	{"
    "		iBoundCoord[5].s = 1 - iBoundCoord[5].s;"
    "		oBoundCoord[5].s = 1 - oBoundCoord[5].s;"
    "	}"
    "	if(u_FlipFlags[5].t == 1)"
    "	{"
    "		iBoundCoord[5].t = 1 - iBoundCoord[5].t;"
    "		oBoundCoord[5].t = 1 - oBoundCoord[5].t;"
    "	}"
    "	iBoundColor[5] = texture2D(u_SamplerIds[5], iBoundCoord[5]);"
    "	oBoundColor[5] = texture2D(u_SamplerIds[5], oBoundCoord[5]);"

    // Calculate Texture Seven's color.
    "	iBoundCoord[6] = gl_TexCoord[6].st;"
    "	oBoundCoord[6] = vec2(gl_TexCoord[6].s, gl_TexCoord[6].t - u_Scales[6].t);"
    "	if(u_FlipFlags[6].s == 1)"
    "	{"
    "		iBoundCoord[6].s = 1 - iBoundCoord[6].s;"
    "		oBoundCoord[6].s = 1 - oBoundCoord[6].s;"
    "	}"
    "	if(u_FlipFlags[6].t == 1)"
    "	{"
    "		iBoundCoord[6].t = 1 - iBoundCoord[6].t;"
    "		oBoundCoord[6].t = 1 - oBoundCoord[6].t;"
    "	}"
    "	iBoundColor[6] = texture2D(u_SamplerIds[6], iBoundCoord[6]);"
    "	oBoundColor[6] = texture2D(u_SamplerIds[6], oBoundCoord[6]);"

    // Calculate Texture Eight's color.
    "	iBoundCoord[7] = gl_TexCoord[7].st;"
    "	oBoundCoord[7] = vec2(gl_TexCoord[7].s, gl_TexCoord[7].t - u_Scales[7].t);"
    "	if(u_FlipFlags[7].s == 1)"
    "	{"
    "		iBoundCoord[7].s = 1 - iBoundCoord[7].s;"
    "		oBoundCoord[7].s = 1 - oBoundCoord[7].s;"
    "	}"
    "	if(u_FlipFlags[7].t == 1)"
    "	{"
    "		iBoundCoord[7].t = 1 - iBoundCoord[7].t;"
    "		oBoundCoord[7].t = 1 - oBoundCoord[7].t;"
    "	}"
    "	iBoundColor[7] = texture2D(u_SamplerIds[7], iBoundCoord[7]);"
    "	oBoundColor[7] = texture2D(u_SamplerIds[7], oBoundCoord[7]);"

    // Calculate Texture Color.
    "	vec4 texColor[8];"

    "	texColor[0] = iBoundColor[0];"
    "	if(gl_TexCoord[0].t > u_Scales[0].t)"
    "	{"
    "		texColor[0] = oBoundColor[0];"
    "	}"

    "	texColor[1] = iBoundColor[1];"
    "	if(gl_TexCoord[1].t > u_Scales[1].t)"
    "	{"
    "		texColor[1] = oBoundColor[1];"
    "	}"

    "	texColor[2] = iBoundColor[2];"
    "	if(gl_TexCoord[2].t > u_Scales[2].t)"
    "	{"
    "		texColor[2] = oBoundColor[2];"
    "	}"

    "	texColor[3] = iBoundColor[3];"
    "	if(gl_TexCoord[3].t > u_Scales[3].t)"
    "	{"
    "		texColor[3] = oBoundColor[3];"
    "	}"

    "	texColor[4] = iBoundColor[4];"
    "	if(gl_TexCoord[4].t > u_Scales[4].t)"
    "	{"
    "		texColor[4] = oBoundColor[4];"
    "	}"

    "	texColor[5] = iBoundColor[5];"
    "	if(gl_TexCoord[5].t > u_Scales[5].t)"
    "	{"
    "		texColor[5] = oBoundColor[5];"
    "	}"

    "	texColor[6] = iBoundColor[6];"
    "	if(gl_TexCoord[6].t > u_Scales[6].t)"
    "	{"
    "		texColor[6] = oBoundColor[6];"
    "	}"

    "	texColor[7] = iBoundColor[7];"
    "	if(gl_TexCoord[7].t > u_Scales[7].t)"
    "	{"
    "		texColor[7] = oBoundColor[7];"
    "	}"

    "	vec4 layered_Color;"
    "	layered_Color = mix(gl_Color, texColor[0], min(texColor[0].a, u_TexAlphas[0]));"
    "	layered_Color = mix(layered_Color, texColor[1], min(texColor[1].a, u_TexAlphas[1]));"
    "	layered_Color = mix(layered_Color, texColor[2], min(texColor[2].a, u_TexAlphas[2]));"
    "	layered_Color = mix(layered_Color, texColor[3], min(texColor[3].a, u_TexAlphas[3]));"
    "	layered_Color = mix(layered_Color, texColor[4], min(texColor[4].a, u_TexAlphas[4]));"
    "	layered_Color = mix(layered_Color, texColor[5], min(texColor[5].a, u_TexAlphas[5]));"
    "	layered_Color = mix(layered_Color, texColor[6], min(texColor[6].a, u_TexAlphas[6]));"
    "	layered_Color = mix(layered_Color, texColor[7], min(texColor[7].a, u_TexAlphas[7]));"

    "	gl_FragColor = layered_Color * v_LightColor;"
    "}";
}
#endif
