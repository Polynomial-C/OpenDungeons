#include <iostream>

#include <OgreStringConverter.h>

#include "Globals.h"
//#include "Functions.h"
#include "Field.h"
#include "RenderRequest.h"
#include "RenderManager.h"

Field::Field(const std::string& nName) :
        hasMeshes(false)
{
    static int uniqueNumber = 0;

    name = (nName.compare("autoname") == 0)
            ? "field_" + Ogre::StringConverter::toString(++uniqueNumber)
            : nName;
}

/*! \brief Returns the stored value at a position (or 0) and a boolean indicating whether the value was actually found.
 *
 */
std::pair<double, bool> Field::get(int x, int y)
{
    LocationType location(x, y);
    FieldType::iterator itr;
    itr = theField.find(location);
    bool found = (itr != theField.end());

    return std::pair<double, bool> (found ? (*itr).second : 0.0, found);
}

FieldType::iterator Field::begin()
{
    return theField.begin();
}

FieldType::iterator Field::end()
{
    return theField.end();
}

/*! \brief Sets the field value at location (x,y) to the value f adding that place if necessary.
 *
 */
void Field::set(int x, int y, double f)
{
    LocationType location(x, y);
    theField[location] = f;
}

/*! \brief Loops over all the places currently set and sets each one to f.
 *
 */
void Field::setAll(double f)
{
    for (FieldType::iterator itr = theField.begin(), end = theField.end();
            itr != end; ++itr)
    {
        (*itr).second = f;
    }
}

/*! \brief Adds the values in f to the values stored in theField.
 *
 * Nonexistant tiles in theField are treated as if their original value was
 * zero (i.e. a new entry is created and set the f(x, y)).
 */
void Field::addField(Field *f, double scale = 1.0)
{
    FieldType::iterator thisOne;
    for (FieldType::iterator itr = f->theField.begin(), end = f->theField.end();
            itr != end; ++itr)
    {
        double fValue = (*itr).second;
        thisOne = theField.find(
                LocationType((*itr).first.first, (*itr).first.second));
        if (thisOne != theField.end())
        {
            (*thisOne).second += scale * fValue;
        }
        else
        {
            theField[(*itr).first] = scale * fValue;
        }
    }
}

void Field::subtractField(Field *f, double scale = 1.0)
{
    addField(f, -1.0 * scale);
}

void Field::clear()
{
    theField.clear();
}

std::pair<LocationType, double> Field::min()
{
    if (theField.empty())
    {
        std::cerr
                << "\n\nERROR:  Trying to find the minumum value on a field of 0 elements.\n\n";
        exit(1);
    }

    FieldType::iterator itr = theField.begin();
    FieldType::iterator minimum = theField.begin();
    while (itr != theField.end())
    {
        if (itr->second < minimum->second)
        {
            minimum = itr;
        }

        ++itr;
    }

    return *minimum;
}

std::pair<LocationType, double> Field::max()
{
    if (theField.empty())
    {
        std::cerr
                << "\n\nERROR:  Trying to find the minumum value on a field of 0 elements.\n\n";
        exit(1);
    }

    FieldType::iterator itr = theField.begin();
    FieldType::iterator maximum = theField.begin();
    while (itr != theField.end())
    {
        if (itr->second > maximum->second)
        {
            maximum = itr;
        }

        ++itr;
    }

    return *maximum;
}
void Field::refreshMeshes(double offset = 0.0)
{
    if (theField.empty() && !hasMeshes)
    {
        return;
    }

    if (!hasMeshes)
    {
        createMeshes(offset);
    }
    else
    {
        hasMeshes = true;

        RenderRequest *request = new RenderRequest;
        request->type = RenderRequest::refreshField;
        request->p = static_cast<void*>(this);
        request->p2 = new double(offset); //FIXME: This leaks memory, it should either be deleted in the request handler or passed via a different method.

        // Add the request to the queue of rendering operations to be performed before the next frame.
        RenderManager::queueRenderRequest(request);
    }
}

void Field::createMeshes(double offset = 0.0)
{
    if (theField.empty() || hasMeshes)
    {
        return;
    }

    hasMeshes = true;

    RenderRequest *request = new RenderRequest;
    request->type = RenderRequest::createField;
    request->p = static_cast<void*>(this);
    request->p2 = new double(offset);

    // Add the request to the queue of rendering operations to be performed before the next frame.
    RenderManager::queueRenderRequest(request);
}

void Field::destroyMeshes()
{
    if (!hasMeshes)
    {
        return;
    }
    hasMeshes = false;

    RenderRequest *request = new RenderRequest;
    request->type = RenderRequest::destroyField;
    request->p = static_cast<void*>(this);

    // Add the request to the queue of rendering operations to be performed before the next frame.
    RenderManager::queueRenderRequest(request);
}

