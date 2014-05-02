#if !defined(VSP_VEHICLEGUIDRAW_INCLUDED__)
#define VSP_VEHICLEGUIDRAW_INCLUDED__

class Labels;
class Lights;

/*!
* Centralized place to access all GUI related Parm objects.
*/
class VehicleGuiDraw
{
public:
    /*!
    * Constructor.
    */
    VehicleGuiDraw();
    /*!
    * Destructor.
    */
    ~VehicleGuiDraw();

public:
    /*!
    * Get Labels pointer.
    */
    Labels * getLabels();
    /*!
    * Get Lights pointer.
    */
    Lights * getLights();
};
#endif