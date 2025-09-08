#pragma once


// GIS DATA RESOURCE TYPES
class IBaseGISDataResource {
public:
	virtual ~IBaseGISDataResource() {}
	virtual bool IsLoaded() const = 0;
};

template<typename T>
class TGISData : public IBaseGISDataResource
{
	T* Data;
public:
	TGISData(T* InData = nullptr) : Data(InData) {}
	virtual ~TGISData() { delete Data; }

	virtual bool IsLoaded() const override { return Data != nullptr; }
	T* GetData() const { return Data; }
	void SetData(T* InData)
	{
		if (Data != InData) {
			delete Data;
			Data = InData;
		}
	}
};