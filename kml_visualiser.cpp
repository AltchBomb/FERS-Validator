// FERS input Validator Function sub-system
// Outputs KML GIS readable file.
// Script written by Michael Altshuler 
// University of Cape Town: ALTMIC003

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>

using namespace std;
using namespace xercesc;

// Function returns coordinates from positionWayPoint values
std::string getCoordinatesFromPositionWaypoint(const DOMElement* positionWaypointElement, double referenceLatitude, double referenceLongitude, double referenceAltitude) {
    const XMLCh* xTag = XMLString::transcode("x");
    const XMLCh* yTag = XMLString::transcode("y");
    const XMLCh* altitudeTag = XMLString::transcode("altitude");

    double x = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(xTag)->item(0)->getTextContent()));
    double y = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(yTag)->item(0)->getTextContent()));
    double altitude = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(altitudeTag)->item(0)->getTextContent()));

    double longitude = referenceLongitude + x / (cos(referenceLatitude * M_PI / 180) * 111319.9);
    double latitude = referenceLatitude + y / 111319.9;
    double altitudeAboveGround = altitude - referenceAltitude;

    std::stringstream coordinates;
    coordinates << std::fixed << std::setprecision(6) << longitude << "," << latitude << "," << altitudeAboveGround;

    return coordinates.str();
}

// Function calculates the hyperbolic path and updates the longitude and latitude values. *Not a valid interpolation path at this time (04/05/2023)
void updateLongitudeLatitudeHyperbolic(double& longitude, double& latitude, double t, double a, double b) {
    double x_hyperbolic = a * std::cosh(t);
    double y_hyperbolic = b * std::sinh(t);

    // Assuming the start point of the hyperbolic path is at the origin
    longitude += x_hyperbolic / (cos(latitude * M_PI / 180) * 111319.9);
    latitude += y_hyperbolic / 111319.9;
}

// Older implementation (deprecated)
void updateLongitudeLatitudeCubic2(double &x, double &y, double t, double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4) {
    double t2 = t * t;
    double t3 = t2 * t;

    double h00 = 2.0 * t3 - 3.0 * t2 + 1.0;
    double h10 = -2.0 * t3 + 3.0 * t2;
    double h01 = t3 - 2.0 * t2 + t;
    double h11 = t3 - t2;

    x = h00 * x1 + h10 * x2 + h01 * x3 + h11 * x4;
    y = h00 * y1 + h10 * y2 + h01 * y3 + h11 * y4;
}

// Function calculates the cubic path and updates the longitude and latitude values.
void updateLongitudeLatitudeCubic(double &newLongitude, double &newLatitude, double t, double longitude1, double latitude1, double longitude4, double latitude4) {
    double t2 = t * t;
    double t3 = t2 * t;

    double controlPointAngle = 45.0 * M_PI / 180.0; // 45 degrees in radians
    double controlPointDistance = 111319.9; // Fixed distance for control points (e.g. 111319.9 meters or 1 degree)

    // Calculate control points based on fixed angle and distance
    double x2 = longitude1 + controlPointDistance * cos(controlPointAngle) / (cos(latitude1 * M_PI / 180) * 111319.9);
    double y2 = latitude1 + controlPointDistance * sin(controlPointAngle) / 111319.9;

    double x3 = longitude4 - controlPointDistance * cos(controlPointAngle) / (cos(latitude4 * M_PI / 180) * 111319.9);
    double y3 = latitude4 - controlPointDistance * sin(controlPointAngle) / 111319.9;

    // Perform cubic interpolation
    double one_minus_t = 1 - t;
    double one_minus_t2 = one_minus_t * one_minus_t;
    double one_minus_t3 = one_minus_t2 * one_minus_t;

    newLongitude = one_minus_t3 * longitude1 + 3 * one_minus_t2 * t * x2 + 3 * one_minus_t * t2 * x3 + t3 * longitude4;
    newLatitude = one_minus_t3 * latitude1 + 3 * one_minus_t2 * t * y2 + 3 * one_minus_t * t2 * y3 + t3 * latitude4;
}

void processElement(const DOMElement* element, std::ofstream& kmlFile, double referenceLatitude, double referenceLongitude, double referenceAltitude) {

    // Defining constants
    const XMLCh* platformTag = XMLString::transcode("platform");
    const XMLCh* receiverTag = XMLString::transcode("receiver");
    const XMLCh* transmitterTag = XMLString::transcode("transmitter");
    const XMLCh* targetTag = XMLString::transcode("target");
    const XMLCh* positionWaypointTag = XMLString::transcode("positionwaypoint");
    const XMLCh* xTag = XMLString::transcode("x");
    const XMLCh* yTag = XMLString::transcode("y");
    const XMLCh* altitudeTag = XMLString::transcode("altitude");
    const XMLCh* motionPathTag = XMLString::transcode("motionpath");
    const XMLCh* interpolationAttr = XMLString::transcode("interpolation");

    // Check if the element is a platform
    if (XMLString::equals(element->getTagName(), platformTag)) {
        // Get the positionwaypoint element
        // Check if the getElementsByTagName() function returns a valid result before using it
        DOMNodeList* positionWaypointList = element->getElementsByTagName(positionWaypointTag);
        if (positionWaypointList->getLength() == 0) {
            return;
        }

        const DOMElement* positionWaypointElement = dynamic_cast<const DOMElement*>(element->getElementsByTagName(positionWaypointTag)->item(0));

        // Extract the position coordinates
        double x = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(xTag)->item(0)->getTextContent()));
        double y = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(yTag)->item(0)->getTextContent()));
        double altitude = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(altitudeTag)->item(0)->getTextContent()));

        // Rough estimation equirectangular projection method.
        double longitude = referenceLongitude + x / (cos(referenceLatitude * M_PI / 180) * 111319.9);
        double latitude = referenceLatitude + y / 111319.9;
        double altitudeAboveGround = altitude - referenceAltitude;

        // Get the motionpath element
        const DOMElement* motionPathElement = dynamic_cast<const DOMElement*>(element->getElementsByTagName(motionPathTag)->item(0));

        // Extract the interpolation attribute
        const XMLCh* interpolation = motionPathElement->getAttribute(interpolationAttr);

        // Determine if the interpolation is linear, hyperbolic or cubic
        bool isLinear = (XMLString::equals(interpolation, XMLString::transcode("linear")));
        bool isHyperbolic = (XMLString::equals(interpolation, XMLString::transcode("hyperbolic")));
        bool isCubic = (XMLString::equals(interpolation, XMLString::transcode("cubic")));

        // Determine the type of placemark to use
        std::string placemarkStyle;
        if (element->getElementsByTagName(receiverTag)->getLength() > 0) {
            placemarkStyle = "receiver";
        } else if (element->getElementsByTagName(transmitterTag)->getLength() > 0) {
            placemarkStyle = "transmitter";
        } else if (element->getElementsByTagName(targetTag)->getLength() > 0) {
            placemarkStyle = "target";
        }

        // Write the placemark data to the KML file
        kmlFile << "<Placemark>\n";
        kmlFile << "    <name>" << XMLString::transcode(element->getAttribute(XMLString::transcode("name"))) << "</name>\n";
        kmlFile << "    <description>" << XMLString::transcode(element->getAttribute(XMLString::transcode("description"))) << "</description>\n";

        if (element->getElementsByTagName(receiverTag)->getLength() > 0) {
            kmlFile << "    <styleUrl>#receiver</styleUrl>\n";
        } else if (element->getElementsByTagName(transmitterTag)->getLength() > 0) {
            kmlFile << "    <styleUrl>#transmitter</styleUrl>\n";
        } else if (element->getElementsByTagName(targetTag)->getLength() > 0) {
            kmlFile << "    <styleUrl>#target</styleUrl>\n";
        }

        // If the interpolation is linear, hyperbolic or exponential, use the gx:Track element
        if (isLinear || isHyperbolic || isCubic) {
            kmlFile << "    <gx:Track>\n";
            if (altitudeAboveGround > 0) {
                kmlFile << "        <altitudeMode>relativeToGround</altitudeMode>\n";
                kmlFile << "        <extrude>1</extrude>\n";
            } else {
                kmlFile << "        <altitudeMode>clampToGround</altitudeMode>\n";
            }

            // Iterate through the position waypoints
            for (XMLSize_t i = 0; i < positionWaypointList->getLength(); ++i) {
                const DOMElement* positionWaypointElement = dynamic_cast<const DOMElement*>(positionWaypointList->item(i));

                // Extract the position coordinates
                double x = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(xTag)->item(0)->getTextContent()));
                double y = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(yTag)->item(0)->getTextContent()));
                double altitude = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(altitudeTag)->item(0)->getTextContent()));

                // Convert the position coordinates to geographic coordinates
                double longitude = referenceLongitude + x / (cos(referenceLatitude * M_PI / 180) * 111319.9);
                double latitude = referenceLatitude + y / 111319.9;
                double altitudeAboveGround = altitude - referenceAltitude;

                // Extract the time value
                const XMLCh* timeTag = XMLString::transcode("time");
                double time = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(timeTag)->item(0)->getTextContent()));

                // Check if interpolation is hyperbolic
                if (isHyperbolic) {
                    // Calculate the hyperbolic path and update longitude and latitude values accordingly
                    double a = 0.5; // Set the desired value for 'a' based on the shape of the hyperbola
                    double b = 0.5; // Set the desired value for 'b' based on the shape of the hyperbola
                    double t = (double)i / (positionWaypointList->getLength() - 1) * 2.0 * M_PI; // Parameter 't' varies from 0 to 2 * PI
                    updateLongitudeLatitudeHyperbolic(longitude, latitude, t, a, b);
                }

                // Check if interpolation is cubic
                if (isCubic && i + 1 < positionWaypointList->getLength()) {
                    // Calculate time difference between two consecutive position waypoints
                    const DOMElement* nextPositionWaypointElement = dynamic_cast<const DOMElement*>(positionWaypointList->item(i + 1));
                    double nextTime = std::stod(XMLString::transcode(nextPositionWaypointElement->getElementsByTagName(timeTag)->item(0)->getTextContent()));
                    double time_diff = nextTime - time;

                    // Extract the position coordinates for the next waypoint
                    double nextX = std::stod(XMLString::transcode(nextPositionWaypointElement->getElementsByTagName(xTag)->item(0)->getTextContent()));
                    double nextY = std::stod(XMLString::transcode(nextPositionWaypointElement->getElementsByTagName(yTag)->item(0)->getTextContent()));
                    double nextAltitude = std::stod(XMLString::transcode(nextPositionWaypointElement->getElementsByTagName(altitudeTag)->item(0)->getTextContent()));

                    // Convert the position coordinates to geographic coordinates
                    double nextLongitude = referenceLongitude + nextX / (cos(referenceLatitude * M_PI / 180) * 111319.9);
                    double nextLatitude = referenceLatitude + nextY / 111319.9;
                    double nextAltitudeAboveGround = nextAltitude - referenceAltitude;

                    // Calculate control points for cubic interpolation
                    double x1 = longitude;
                    double y1 = latitude;
                    double x4 = nextLongitude;
                    double y4 = nextLatitude;
                    double newX, newY;
                    updateLongitudeLatitudeCubic(newX, newY, 0.0, x1, y1, x4, y4); // Calculate first point on cubic curve

                    int num_divisions = 100;
                    for (int j = 0; j <= num_divisions; ++j) {
                        double t = (double)j / num_divisions;

                        double newLongitude, newLatitude;
                        updateLongitudeLatitudeCubic(newLongitude, newLatitude, t, x1, y1, x4, y4);

                        double newAltitudeAboveGround = altitudeAboveGround + t * (nextAltitudeAboveGround - altitudeAboveGround);

                        kmlFile << "        <when>" << time + (double)(j * time_diff) / num_divisions << "</when>\n";
                        kmlFile << "        <gx:coord>" << newLongitude << " " << newLatitude << " " << newAltitudeAboveGround << "</gx:coord>\n";
                    }
                }
            
        
                // Older implementation that makes use of cubic function that isn't contained within start and end points.
                /*
                // If the interpolation is linear, hyperbolic or exponential, use the gx:Track element
                if (isLinear || isHyperbolic || isCubic) {
                    kmlFile << "    <gx:Track>\n";
                    if (altitudeAboveGround > 0) {
                        kmlFile << "        <altitudeMode>relativeToGround</altitudeMode>\n";
                        kmlFile << "        <extrude>1</extrude>\n";
                    } else {
                        kmlFile << "        <altitudeMode>clampToGround</altitudeMode>\n";
                    }

                    // Iterate through the position waypoints
                    for (XMLSize_t i = 0; i < positionWaypointList->getLength(); ++i) {
                        const DOMElement* positionWaypointElement = dynamic_cast<const DOMElement*>(positionWaypointList->item(i));

                        // Extract the position coordinates
                        double x = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(xTag)->item(0)->getTextContent()));
                        double y = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(yTag)->item(0)->getTextContent()));
                        double altitude = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(altitudeTag)->item(0)->getTextContent()));

                        // Convert the position coordinates to geographic coordinates
                        double longitude = referenceLongitude + x / (cos(referenceLatitude * M_PI / 180) * 111319.9);
                        double latitude = referenceLatitude + y / 111319.9;
                        double altitudeAboveGround = altitude - referenceAltitude;

                        // Extract the time value
                        const XMLCh* timeTag = XMLString::transcode("time");
                        double time = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(timeTag)->item(0)->getTextContent()));

                        // Check if interpolation is hyperbolic
                        if (isHyperbolic) {
                            // Calculate the hyperbolic path and update longitude and latitude values accordingly
                            double a = 0.5; // Set the desired value for 'a' based on the shape of the hyperbola
                            double b = 0.5; // Set the desired value for 'b' based on the shape of the hyperbola
                            double t = (double)i / (positionWaypointList->getLength() - 1) * 2.0 * M_PI; // Parameter 't' varies from 0 to 2 * PI
                            updateLongitudeLatitudeHyperbolic(longitude, latitude, t, a, b);
                        }
                        
                        // Check if interpolation is cubic
                        if (isCubic && i + 1 < positionWaypointList->getLength()) {
                            // Calculate time difference between two consecutive position waypoints
                            const DOMElement* nextPositionWaypointElement = dynamic_cast<const DOMElement*>(positionWaypointList->item(i + 1));
                            double nextTime = std::stod(XMLString::transcode(nextPositionWaypointElement->getElementsByTagName(timeTag)->item(0)->getTextContent()));
                            double time_diff = nextTime - time;

                            // Extract the position coordinates for the next waypoint
                            double nextX = std::stod(XMLString::transcode(nextPositionWaypointElement->getElementsByTagName(xTag)->item(0)->getTextContent()));
                            double nextY = std::stod(XMLString::transcode(nextPositionWaypointElement->getElementsByTagName(yTag)->item(0)->getTextContent()));
                            double nextAltitude = std::stod(XMLString::transcode(nextPositionWaypointElement->getElementsByTagName(altitudeTag)->item(0)->getTextContent()));

                            // Calculate next longitude, latitude, and altitude above ground
                            double nextLongitude = referenceLongitude + nextX / (cos(referenceLatitude * M_PI / 180) * 111319.9);
                            double nextLatitude = referenceLatitude + nextY / 111319.9;
                            double nextAltitudeAboveGround = nextAltitude - referenceAltitude;

                            // Calculate control points for cubic interpolation
                            double x2 = longitude + (nextLongitude - longitude) / 3;
                            double y2 = latitude + (nextLatitude - latitude) / 3;
                            double x3 = nextLongitude - (nextLongitude - longitude) / 3;
                            double y3 = nextLatitude - (nextLatitude - latitude) / 3;

                            int num_divisions = 100;
                            for (int j = 0; j <= num_divisions; ++j) {
                                double t = (double)j / num_divisions;

                                double newX, newY;
                                updateLongitudeLatitudeCubic(newX, newY, t, longitude, latitude, x2, y2, x3, y3, nextLongitude, nextLatitude);
                                double newAltitudeAboveGround = altitudeAboveGround + t * (nextAltitudeAboveGround - altitudeAboveGround);

                                kmlFile << "        <when>" << time + (double)(j * time_diff) / num_divisions << "</when>\n";
                                kmlFile << "        <gx:coord>" << newX << " " << newY << " " << newAltitudeAboveGround << "</gx:coord>\n";
                            }
                        } */

                else {
                    // Write the time and coordinates to the gx:Track element
                    kmlFile << "        <when>" << time << "</when>\n";
                    kmlFile << "        <gx:coord>" << longitude << " " << latitude << " " << altitudeAboveGround << "</gx:coord>\n";
                }
            }

            kmlFile << "    </gx:Track>\n";
        }

        else {
            kmlFile << "    <LookAt>\n";
            kmlFile << "        <longitude>" << longitude << "</longitude>\n";
            kmlFile << "        <latitude>" << latitude << "</latitude>\n";
            kmlFile << "        <altitude>" << altitudeAboveGround << "</altitude>\n";
            kmlFile << "        <heading>-148.4122922628044</heading>\n";
            kmlFile << "        <tilt>40.5575073395506</tilt>\n";
            kmlFile << "        <range>500.6566641072245</range>\n";
            kmlFile << "    </LookAt>\n";

            kmlFile << "    <Point>\n";
            kmlFile << "        <coordinates>" << longitude << "," << latitude << "," << altitudeAboveGround << "</coordinates>\n";

            if (altitudeAboveGround > 0) {
                kmlFile << "        <altitudeMode>relativeToGround</altitudeMode>\n";
                kmlFile << "        <extrude>1</extrude>\n";
            } else {
                kmlFile << "        <altitudeMode>clampToGround</altitudeMode>\n";
            }

            kmlFile << "    </Point>\n"; 
        }

        kmlFile << "</Placemark>\n";

        if (isLinear || isHyperbolic || isCubic) {
            // Get the first and last position waypoints
            const DOMElement* firstPositionWaypointElement = dynamic_cast<const DOMElement*>(positionWaypointList->item(0));
            const DOMElement* lastPositionWaypointElement = dynamic_cast<const DOMElement*>(positionWaypointList->item(positionWaypointList->getLength() - 1));

            // Extract the start and end coordinates
            std::string startCoordinates = getCoordinatesFromPositionWaypoint(firstPositionWaypointElement, referenceLatitude, referenceLongitude, referenceAltitude);
            std::string endCoordinates = getCoordinatesFromPositionWaypoint(lastPositionWaypointElement, referenceLatitude, referenceLongitude, referenceAltitude);

            // Start point placemark
            kmlFile << "<Placemark>\n";
            kmlFile << "    <name>Start: " << XMLString::transcode(element->getAttribute(XMLString::transcode("name"))) << "</name>\n";
            kmlFile << "    <styleUrl>#target</styleUrl>\n"; // Replace with your desired style URL for the start icon
            kmlFile << "    <Point>\n";
            kmlFile << "        <coordinates>" << startCoordinates << "</coordinates>\n";
            if (altitudeAboveGround > 0) {
                kmlFile << "        <altitudeMode>relativeToGround</altitudeMode>\n";
                kmlFile << "        <extrude>1</extrude>\n";
            } else {
                kmlFile << "        <altitudeMode>clampToGround</altitudeMode>\n";
            }
            kmlFile << "    </Point>\n";
            kmlFile << "</Placemark>\n";

            // End point placemark
            kmlFile << "<Placemark>\n";
            kmlFile << "    <name>End: " << XMLString::transcode(element->getAttribute(XMLString::transcode("name"))) << "</name>\n";
            kmlFile << "    <styleUrl>#target</styleUrl>\n"; // Replace with your desired style URL for the end icon
            kmlFile << "    <Point>\n";
            kmlFile << "        <coordinates>" << endCoordinates << "</coordinates>\n";
            if (altitudeAboveGround > 0) {
                kmlFile << "        <altitudeMode>relativeToGround</altitudeMode>\n";
                kmlFile << "        <extrude>1</extrude>\n";
            } else {
                kmlFile << "        <altitudeMode>clampToGround</altitudeMode>\n";
            }
            kmlFile << "    </Point>\n";
            kmlFile << "</Placemark>\n";
        }

    }
}

void traverseDOMNode(const DOMNode* node, std::ofstream& kmlFile, double referenceLatitude, double referenceLongitude, double referenceAltitude) {
    if (node->getNodeType() == DOMNode::ELEMENT_NODE) {
        const DOMElement* element = dynamic_cast<const DOMElement*>(node);
        processElement(element, kmlFile, referenceLatitude, referenceLongitude, referenceAltitude);
    }

    for (DOMNode* child = node->getFirstChild(); child != nullptr; child = child->getNextSibling()) {
        traverseDOMNode(child, kmlFile, referenceLatitude, referenceLongitude, referenceAltitude);
    }
}

// Main function
int main(int argc, char* argv[]) {

    if (argc > 3 && argc < 6) {
        std::cerr << "Usage: " << argv[0] << " <input XML file> <output KML file> [<referenceLatitude> <referenceLongitude> <referenceAltitude>]" << std::endl;
        return 1;
    }

    // Setting default geographical and altitude coordinates
    double referenceLatitude = -33.9545;
    double referenceLongitude = 18.4563;
    double referenceAltitude = 0;

    // Update file_path with command line argument
    string file_path = argv[1];
    // Setting mode to evironment variable from command line.
    string output_file = argv[2];
    // Setting georgraphical coordinates to command line input
    if (argc == 6) {
        try {
            referenceLatitude = std::stod(argv[3]);
            referenceLongitude = std::stod(argv[4]);
            referenceAltitude = std::stod(argv[5]);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: Invalid argument. Please provide valid numbers for referenceLatitude, referenceLongitude, and referenceAltitude.\n";
            return 1;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: Out of range. Please provide valid numbers for referenceLatitude, referenceLongitude, and referenceAltitude.\n";
            return 1;
        }
    }

    try {
        XMLPlatformUtils::Initialize();
        XercesDOMParser parser;

        // Error handler configuration
        ErrorHandler* errorHandler = (ErrorHandler*) new HandlerBase();
        parser.setErrorHandler(errorHandler); 

        parser.setValidationScheme(XercesDOMParser::Val_Never);
        parser.setDoNamespaces(false);
        parser.setDoSchema(false);
        parser.setLoadExternalDTD(false);

        // Use file_path from command line argument
        parser.parse(file_path.c_str()); 

        // Creating DOMDocument and checking if document pointer is valid
        DOMDocument* document = parser.getDocument();
        if (!document) {
            std::cerr << "Error: document not found" << std::endl;
            XMLPlatformUtils::Terminate();
            return 1;
        }

        // Creating rootElement and checking if rootElement pointer is valid 
        DOMElement* rootElement = document->getDocumentElement();
        if (!rootElement) {
            std::cerr << "Error: root element not found" << std::endl;
            XMLPlatformUtils::Terminate();
            return 1;
        }

        std::ofstream kmlFile(output_file.c_str());
        if (!kmlFile.is_open()) {
            std::cerr << "Error opening output KML file" << std::endl;
            XMLPlatformUtils::Terminate();
            return 1;
        }

        // Write the KML header
        kmlFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        kmlFile << "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\">\n";
        kmlFile << "<Document>\n";
        kmlFile << "<name>" << file_path << "</name>\n";

        // Add styles here
        kmlFile << "<Style id=\"receiver\">\n";
        kmlFile << "  <IconStyle>\n";
        kmlFile << "    <Icon>\n";
        kmlFile << "      <href>https://cdn-icons-png.flaticon.com/512/645/645436.png</href>\n";
        kmlFile << "    </Icon>\n";
        kmlFile << "  </IconStyle>\n";
        kmlFile << "</Style>\n";
        kmlFile << "<Style id=\"transmitter\">\n";
        kmlFile << "  <IconStyle>\n";
        kmlFile << "    <Icon>\n";
        kmlFile << "      <href>https://cdn-icons-png.flaticon.com/128/224/224666.png</href>\n";
        kmlFile << "    </Icon>\n";
        kmlFile << "  </IconStyle>\n";
        kmlFile << "</Style>\n";
        kmlFile << "<Style id=\"target\">\n";
        kmlFile << "  <IconStyle>\n";
        kmlFile << "    <Icon>\n";
        kmlFile << "      <href>https://upload.wikimedia.org/wikipedia/commons/thumb/a/ad/Target_red_dot1.svg/1200px-Target_red_dot1.svg.png</href>\n";
        kmlFile << "    </Icon>\n";
        kmlFile << "  </IconStyle>\n";
        kmlFile << "  <LineStyle>\n";
        kmlFile << "    <width>2</width>\n";
        kmlFile << "  </LineStyle>\n";
        kmlFile << "</Style>\n";

        // Add the Folder element
        kmlFile << "<Folder>\n";
        kmlFile << "  <name>Reference Coordinate</name>\n";
        kmlFile << "  <description>Placemarks for various elements in the FERSXML file. All Placemarks are situated relative to this reference point.</description>\n";

        // Add the LookAt element with given values
        kmlFile << "  <LookAt>\n";
        kmlFile << "    <longitude>" << referenceLongitude << "</longitude>\n";
        kmlFile << "    <latitude>"<< referenceLatitude << "</latitude>\n";
        kmlFile << "    <altitude>" << referenceAltitude << "</altitude>\n";
        kmlFile << "    <heading>-148.4122922628044</heading>\n";
        kmlFile << "    <tilt>40.5575073395506</tilt>\n";
        kmlFile << "    <range>10000</range>\n";
        kmlFile << "  </LookAt>\n";

        // Traverse DOMNode and output extracted FERSXML data:
        traverseDOMNode(rootElement, kmlFile, referenceLatitude, referenceLongitude, referenceAltitude);

        // Close the Folder and Document elements
        kmlFile << "</Folder>\n";
        kmlFile << "</Document>\n";
        kmlFile << "</kml>\n";

        kmlFile.close();
        
        delete errorHandler; // Clean up the error handler
    }
    catch (const XMLException& e) {
        cerr << "Error initializing Xerces-C++: " << XMLString::transcode(e.getMessage()) << endl;
        return 1;
    }
    catch (const DOMException& e) {
        cerr << "Error parsing XML: " << XMLString::transcode(e.getMessage()) << endl;
        return 1;
    }
    catch (const SAXException& e) {
        cerr << "Error parsing XML: " << XMLString::transcode(e.getMessage()) << endl;
        return 1;
    }
    catch (...) {
        cerr << "Unknown error occurred while parsing XML." << endl;
        return 1;
    }

    XMLPlatformUtils::Terminate();
    return 0;
}