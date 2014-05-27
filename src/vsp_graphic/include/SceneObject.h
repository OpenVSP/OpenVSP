#ifndef _VSP_GRAPHIC_SCENE_OBJECT_H
#define _VSP_GRAPHIC_SCENE_OBJECT_H

namespace VSPGraphic
{
/*
* Base class for all "Scene" object types.
*/
class SceneObject
{
public:
	/*!
	* Constructor.
	*/
	SceneObject();
	/*!
	* Destructor.
	*/
	virtual ~SceneObject();

public:
	/*!
	* Preprocessing rendering.
	*/
	void predraw();
	/*!
	* Render this scene object.
	*/
	void draw();

public:
	/*!
	* Set visibility of this object.  True to show, false to hide.
	*/
	virtual void setVisibility( bool visible );

public:
	/*!
	* Get id for this object.  This object does not generate ids, 
	* this id must be set.  If id not set, return 0xFFFFFFFF.
	*/
	unsigned int getID();
	/*!
	* Set id.
	*/
	void setID( unsigned int id );

protected:
	/*!
	* Perform preprocessing.
	* All pre-render / render passes goes here.  Must implement. 
	*/
	virtual void _predraw() = 0;
	/*!
	* Draw object.  
	* All actual rendering code goes here.  Must implement.
	*/
	virtual void _draw() = 0;

protected:
	/*!
	* Enable/Disable preprocessing.  Set true to enable, else disable.
	*/
	void enablePredraw(bool enable);

private:
	unsigned int _id;

	bool _visible;
	bool _predrawFlag;
};
}
#endif