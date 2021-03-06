/**********************************************************************
Copyright 2020 Advanced Micro Devices, Inc
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
********************************************************************/
#include "FireMaya.h"
#include "PhysicalLightAttributes.h"
#include "FireRenderUtils.h"
#include "FireRenderMath.h"

#include <maya/MPxNode.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

MObject PhysicalLightAttributes::enabled;
MObject PhysicalLightAttributes::lightType;
MObject PhysicalLightAttributes::areaLength;
MObject PhysicalLightAttributes::areaWidth;
MObject PhysicalLightAttributes::targeted;

// Intensity
MObject PhysicalLightAttributes::lightIntensity;
MObject PhysicalLightAttributes::intensityUnits;
MObject PhysicalLightAttributes::luminousEfficacy;
MObject PhysicalLightAttributes::colorMode;
MObject PhysicalLightAttributes::colorPicker;
MObject PhysicalLightAttributes::temperature;
MObject PhysicalLightAttributes::temperatureColor;

// Area Light
MObject PhysicalLightAttributes::areaLightVisible;
MObject PhysicalLightAttributes::areaLightBidirectional;
MObject PhysicalLightAttributes::areaLightShape;
MObject PhysicalLightAttributes::areaLightSelectingMesh;
MObject PhysicalLightAttributes::areaLightMeshSelectedName;
MObject PhysicalLightAttributes::areaLightIntensityNorm;

// Spot Light
MObject PhysicalLightAttributes::spotLightVisible;
MObject PhysicalLightAttributes::spotLightInnerConeAngle;
MObject PhysicalLightAttributes::spotLightOuterConeFalloff;

// Sphere Light
MObject PhysicalLightAttributes::sphereLightRadius;

// Disk light
MObject PhysicalLightAttributes::diskLightRadius;
MObject PhysicalLightAttributes::diskLightAngle;

//Light Decay
MObject PhysicalLightAttributes::decayType;
MObject PhysicalLightAttributes::decayFalloffStart;
MObject PhysicalLightAttributes::decayFalloffEnd;

// Shadows
MObject PhysicalLightAttributes::shadowsEnabled;
MObject PhysicalLightAttributes::shadowsSoftnessAngle;
MObject PhysicalLightAttributes::shadowsTransparency;

// Volume
MObject PhysicalLightAttributes::volumeScale;

void CreateIntAttribute(MObject& propObject, const char* name, const char* shortName, int minVal, int maxVal, int defaultVal)
{
	MFnNumericAttribute nAttr;

	propObject = nAttr.create(name, shortName, MFnNumericData::kInt, defaultVal);

	nAttr.setMin(minVal);
	nAttr.setSoftMax(maxVal);
	setAttribProps(nAttr, propObject);
}

void CreateFloatAttribute(MObject& propObject, const char* name, const char* shortName, float minVal, float maxVal, float defaultVal)
{
	MFnNumericAttribute nAttr;

	propObject = nAttr.create(name, shortName, MFnNumericData::kFloat, defaultVal);

	nAttr.setMin(minVal);
	nAttr.setSoftMax(maxVal);
	setAttribProps(nAttr, propObject);
}

void CreateBoolAttribute(MObject& propObject, const char* name, const char* shortName, bool defaultVal)
{
	MFnNumericAttribute nAttr;

	propObject = nAttr.create(name, shortName, MFnNumericData::kBoolean, defaultVal);

	setAttribProps(nAttr, propObject);
}

void CreateEnumAttribute(MObject& propObject, const char* name, const char* shortName, const std::vector<const char*>& values, int defaultIndex)
{
	MFnEnumAttribute eAttr;

	propObject = eAttr.create(name, shortName, defaultIndex);

	for (int i = 0; i < values.size(); ++i)
	{
		eAttr.addField(values[i], i);
	}

	setAttribProps(eAttr, propObject);
}

void CreateColor(MObject& propObject, const char* name, const char* shortName, bool readOnly = false)
{
	MFnNumericAttribute nAttr;

	propObject = nAttr.createColor(name, shortName);
	CHECK_MSTATUS(nAttr.setDefault(1.0f, 1.0f, 1.0f));

	setAttribProps(nAttr, propObject);

	if (readOnly)
	{
		nAttr.setWritable(false);
	}
}

void PhysicalLightAttributes::Initialize()
{
	CreateGeneralAttributes();
	CreateIntensityAttributes();
	CreateAreaLightAttrbutes();
	CreateSphereLightAttrbutes();
	CreateSpotLightAttrbutes();
	CreateDiskLightAttrbutes();
	CreateDecayAttrbutes();
	CreateShadowsAttrbutes();
	CreateVolumeAttrbutes();
	CreateHiddenAttributes();

	CreateLegacyAttributes();
}

void PhysicalLightAttributes::CreateGeneralAttributes()
{
	float maxVal = 10.0f;

	CreateBoolAttribute(enabled, "enabled", "en", true);

	std::vector<const char*> values = { "Area" , "Spot", "Point", "Directional", "Sphere (RPR 2 only)", "Disk (RPR 2 only)"};

	CreateEnumAttribute(lightType, "lightType", "lt", values, PLTArea);

	CreateFloatAttribute(areaLength, "areaLength", "al", 0.0f, maxVal, 1.0f);
	CreateFloatAttribute(areaWidth, "areaWidth", "aw", 0.0f, maxVal, 1.0f);

	CreateBoolAttribute(targeted, "targeted", "td", false);
}

void PhysicalLightAttributes::CreateIntensityAttributes()
{
	float maxVal = 100;

	CreateFloatAttribute(lightIntensity, "intensity", "li", 0.0f, maxVal, 1.0f);

	CreateIntAttribute(intensityUnits, "intensityUnits", "iu", 0, 3, PLTIUWatts);

	CreateFloatAttribute(luminousEfficacy, "luminousEfficacy", "le", 0.1f, maxVal, PhysicalLightData::defaultLuminousEfficacy);

	std::vector<const char*> colorValues = { "Color", "Temperature" };
	CreateEnumAttribute(colorMode, "colorMode", "cm", colorValues, PLCColor);

	CreateColor(colorPicker, "color", "cp");

	CreateFloatAttribute(temperature, "temperature", "t", 0.0f, 40000.0f, 6500.0f);

	CreateColor(temperatureColor, "temperatureColor", "tc", true);
}

void PhysicalLightAttributes::CreateAreaLightAttrbutes()
{
	CreateBoolAttribute(areaLightVisible, "areaLightVisible", "alv", false);
	CreateBoolAttribute(areaLightBidirectional, "areaLightBidirectional", "albd", false);

	std::vector<const char*> values = { "Disc", "Cylinder", "Sphere", "Rectangle", "Mesh" };
	CreateEnumAttribute(areaLightShape, "areaLightShape", "alls", values, PLARectangle);
	CreateBoolAttribute(areaLightIntensityNorm, "areaLightIntensityNorm", "alin", false);
}

void PhysicalLightAttributes::CreateSpotLightAttrbutes()
{
	CreateBoolAttribute(spotLightVisible, "spotLightVisible", "slv", false);

	CreateFloatAttribute(spotLightInnerConeAngle, "spotLightInnerConeAngle", "slia", 0.0f, 179.0f, 43.0f);
	CreateFloatAttribute(spotLightOuterConeFalloff, "spotLightOuterConeFalloff", "slof", 0.0f, 179.0f, 45.0f);
}

void PhysicalLightAttributes::CreateSphereLightAttrbutes()
{
	CreateFloatAttribute(sphereLightRadius, "sphereLightRadius", "slr", 0.0f, 1.0f, 0.1f);
}

void PhysicalLightAttributes::CreateDiskLightAttrbutes()
{
	CreateFloatAttribute(diskLightRadius, "diskLightRadius", "dlr", 0.0f, 10.0f, 0.1f);
	CreateFloatAttribute(diskLightAngle, "diskLightAngle", "dla", 0.0f, 179.0f, 45.0f);
}

void PhysicalLightAttributes::CreateDecayAttrbutes()
{
	float maxVal = 100;

	std::vector<const char*> values = { "None", "InverseSq", "Linear" };
	CreateEnumAttribute(decayType, "decayType", "dt", values, PLDNone);

	CreateFloatAttribute(decayFalloffStart, "decayFalloffStart", "dfs", 0.0f, maxVal, 1.0f);
	CreateFloatAttribute(decayFalloffEnd, "decayFalloffEnd", "dfe", 0.0f, maxVal, 1.0f);
}

void PhysicalLightAttributes::CreateShadowsAttrbutes()
{
	CreateBoolAttribute(shadowsEnabled, "shadowsEnabled", "se", true);

	// give name "sssa" because name "ssa" crashes later for some reason
	CreateFloatAttribute(shadowsSoftnessAngle, "shadowsSoftnessAngle", "sssa", 0.0f, 90.0f, 0.0f);
	MFnNumericAttribute(shadowsSoftnessAngle).setMax(90.0f);
	
	CreateFloatAttribute(shadowsTransparency, "shadowsTransparency", "st", 0.0f, 1.0f, 1.0f);
}

void PhysicalLightAttributes::CreateVolumeAttrbutes()
{
	CreateFloatAttribute(volumeScale, "volumeScale", "vs", 0.0f, 1.0f, 1.0f);
}

void PhysicalLightAttributes::CreateHiddenAttributes()
{
	MFnTypedAttribute tAttr;
	MStatus status;

	CreateBoolAttribute(areaLightSelectingMesh, "areaLightSelectingMesh", "alsm", false);

	areaLightMeshSelectedName = tAttr.create("areaLightMeshSelectedName", "almsn", MFnData::kString, &status);
	tAttr.setHidden(true);
	setAttribProps(tAttr, areaLightMeshSelectedName);
}

void PhysicalLightAttributes::CreateLegacyAttributes()
{
	MObject legacyShadowSoftness;
	CreateFloatAttribute(legacyShadowSoftness, "shadowsSoftness", "ss", 0.0f, 1.0f, 0.02f);
}

int GetIntAttribute(const MFnDependencyNode& node, const MObject& attribute)
{
	MPlug plug = node.findPlug(attribute, false); 

	assert(!plug.isNull());

	if (!plug.isNull()) 
	{
		return plug.asInt();
	}

	return -1;
}

float GetFloatAttribute(const MFnDependencyNode& node, const MObject& attribute)
{
	MPlug plug = node.findPlug(attribute, false);

	MString name1 = MFnNumericAttribute(attribute).name();
	MString name2 = plug.name();

	assert(!plug.isNull());
	if (!plug.isNull())
	{
		return plug.asFloat();
	}

	return 0.0f;
}

bool GetBoolAttribute(const MFnDependencyNode& node, const MObject& attribute)
{
	MPlug plug = node.findPlug(attribute, false);

	assert(!plug.isNull());
	if (!plug.isNull())
	{
		return plug.asBool();
	}

	return false;
}

MString GetStringAttribute(const MFnDependencyNode& node, const MObject& attribute)
{
	MPlug plug = node.findPlug(attribute, false);

	assert(!plug.isNull());
	if (!plug.isNull())
	{
		return plug.asString();
	}

	return "";
}

bool PhysicalLightAttributes::GetEnabled(const MFnDependencyNode& node)
{
	return GetBoolAttribute(node, PhysicalLightAttributes::enabled);
}

PLColorMode PhysicalLightAttributes::GetColorMode(const MFnDependencyNode& node)
{
	return (PLColorMode)GetIntAttribute(node, PhysicalLightAttributes::colorMode);
}

// Get light color for non connected map case
MColor PhysicalLightAttributes::GetColor(const MFnDependencyNode& node)
{
	return getColorAttribute(node, PhysicalLightAttributes::colorPicker);
}

MColor PhysicalLightAttributes::GetTempreratureColor(const MFnDependencyNode& node)
{
	return ConvertKelvinToColor(GetFloatAttribute(node, PhysicalLightAttributes::temperature));
}

float PhysicalLightAttributes::GetIntensity(const MFnDependencyNode& node)
{
	return GetFloatAttribute(node, PhysicalLightAttributes::lightIntensity);
}

float PhysicalLightAttributes::GetLuminousEfficacy(const MFnDependencyNode& node)
{
	return GetFloatAttribute(node, PhysicalLightAttributes::luminousEfficacy);
}

PLIntensityUnit PhysicalLightAttributes::GetIntensityUnits(const MFnDependencyNode& node)
{
	return (PLIntensityUnit) GetIntAttribute(node, PhysicalLightAttributes::intensityUnits);
}

PLAreaLightShape PhysicalLightAttributes::GetAreaLightShape(const MFnDependencyNode& node)
{
	return (PLAreaLightShape) GetIntAttribute(node, PhysicalLightAttributes::areaLightShape);
}

bool PhysicalLightAttributes::GetAreaLightVisible(const MFnDependencyNode& node)
{
	return GetBoolAttribute(node, PhysicalLightAttributes::areaLightVisible);
}

float PhysicalLightAttributes::GetAreaWidth(const MFnDependencyNode& node)
{
	return GetFloatAttribute(node, areaWidth);
}

float PhysicalLightAttributes::GetAreaLength(const MFnDependencyNode& node)
{
	return GetFloatAttribute(node, areaLength);
}

PLType PhysicalLightAttributes::GetLightType(const MFnDependencyNode& node)
{
	return (PLType)GetIntAttribute(node, PhysicalLightAttributes::lightType);
}

void PhysicalLightAttributes::GetSpotLightSettings(const MFnDependencyNode& node, float& innerAngle, float& outerfalloff)
{
	innerAngle = GetFloatAttribute(node, PhysicalLightAttributes::spotLightInnerConeAngle);
	outerfalloff = GetFloatAttribute(node, PhysicalLightAttributes::spotLightOuterConeFalloff);
}

void PhysicalLightAttributes::GetDiskLightSettings(const MFnDependencyNode& node, float& radius, float& angle)
{
	radius = GetFloatAttribute(node, PhysicalLightAttributes::diskLightRadius);
	angle = GetFloatAttribute(node, PhysicalLightAttributes::diskLightAngle);
}

void PhysicalLightAttributes::GetSphereLightSettings(const MFnDependencyNode& node, float& radius)
{
	radius = GetFloatAttribute(node, PhysicalLightAttributes::sphereLightRadius);
}


bool PhysicalLightAttributes::GetShadowsEnabled(const MFnDependencyNode& node)
{
	return GetBoolAttribute(node, PhysicalLightAttributes::shadowsEnabled);
}

float PhysicalLightAttributes::GetShadowsSoftnessAngle(const MFnDependencyNode& node)
{
	return GetFloatAttribute(node, PhysicalLightAttributes::shadowsSoftnessAngle);
}

MString PhysicalLightAttributes::GetAreaLightMeshSelectedName(const MFnDependencyNode& node)
{
	return GetStringAttribute(node, PhysicalLightAttributes::areaLightMeshSelectedName);
}

void PhysicalLightAttributes::FillPhysicalLightData(PhysicalLightData& physicalLightData, const MObject& node, FireMaya::Scope* scope)
{
	physicalLightData.enabled = PhysicalLightAttributes::GetEnabled(node);

	physicalLightData.lightType = PhysicalLightAttributes::GetLightType(node);

	physicalLightData.colorMode = PhysicalLightAttributes::GetColorMode(node);
	physicalLightData.temperatureColorBase = PhysicalLightAttributes::GetTempreratureColor(node);
	physicalLightData.colorBase = PhysicalLightAttributes::GetColor(node);
	physicalLightData.resultFrwColor = frw::Value(physicalLightData.colorBase.r, 
											physicalLightData.colorBase.g, physicalLightData.colorBase.b);

	physicalLightData.intensity = PhysicalLightAttributes::GetIntensity(node);
	physicalLightData.luminousEfficacy = PhysicalLightAttributes::GetLuminousEfficacy(node);
	physicalLightData.intensityUnits = PhysicalLightAttributes::GetIntensityUnits(node);

	physicalLightData.areaWidth = GetAreaWidth(node);
	physicalLightData.areaLength = GetAreaLength(node);

	physicalLightData.shadowsEnabled = GetShadowsEnabled(node);
	physicalLightData.shadowsSoftnessAngle = GetShadowsSoftnessAngle(node);

	if (physicalLightData.lightType == PLTArea)
	{
		physicalLightData.areaLightShape = PhysicalLightAttributes::GetAreaLightShape(node);
		physicalLightData.areaLightVisible = PhysicalLightAttributes::GetAreaLightVisible(node);

		if (scope != nullptr)
		{
			frw::Value frwColor;
			if (physicalLightData.colorMode == PLCColor)
			{
				MPlug plug = MFnDependencyNode(node).findPlug(PhysicalLightAttributes::colorPicker, false);
				frwColor = scope->GetValue(plug);
			}
			else
			{
				MColor color = physicalLightData.temperatureColorBase;
				frwColor = frw::Value(color.r, color.g, color.b);
			}

			physicalLightData.resultFrwColor = frwColor;
		}
	} 
	else if (physicalLightData.lightType == PLTSpot)
	{
		GetSpotLightSettings(node, physicalLightData.spotInnerAngle, physicalLightData.spotOuterFallOff);

		// We should divide by 2 because Core accepts half of an angle value
		physicalLightData.spotInnerAngle = (float) deg2rad(physicalLightData.spotInnerAngle / 2.0f);
		physicalLightData.spotOuterFallOff = (float) deg2rad(physicalLightData.spotOuterFallOff / 2.0f);
	}
	else if (physicalLightData.lightType == PLTDisk)
	{
		physicalLightData.diskRadius = GetFloatAttribute(node, PhysicalLightAttributes::diskLightRadius);
		physicalLightData.diskAngle = (float) deg2rad(GetFloatAttribute(node, PhysicalLightAttributes::diskLightAngle));
	}
	else if (physicalLightData.lightType == PLTSphere)
	{
		physicalLightData.sphereRadius = GetFloatAttribute(node, PhysicalLightAttributes::sphereLightRadius);
	}
}
