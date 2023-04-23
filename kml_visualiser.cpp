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

std::string getCoordinatesFromPositionWaypoint(const DOMElement* positionWaypointElement, double referenceLatitude, double referenceLongitude, double referenceAltitude) {
    const XMLCh* xTag = XMLString::transcode("x");
    const XMLCh* yTag = XMLString::transcode("y");
    const XMLCh* altitudeTag = XMLString::transcode("altitude");

    double x = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(xTag)->item(0)->getTextContent()));
    double y = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(yTag)->item(0)->getTextContent()));
    double altitude = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(altitudeTag)->item(0)->getTextContent()));

    // Convert the position coordinates to geographic coordinates
    // Default coordinates
    // const double referenceLatitude = -33.9545;
    // const double referenceLongitude = 18.4563;
    // const double referenceAltitude = 0;

    double longitude = referenceLongitude + x / (cos(referenceLatitude * M_PI / 180) * 111319.9);
    double latitude = referenceLatitude + y / 111319.9;
    double altitudeAboveGround = altitude - referenceAltitude;

    std::stringstream coordinates;
    coordinates << std::fixed << std::setprecision(6) << longitude << "," << latitude << "," << altitudeAboveGround;

    return coordinates.str();
}


void processElement(const DOMElement* element, std::ofstream& kmlFile, double referenceLatitude, double referenceLongitude, double referenceAltitude) {
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

        // Convert the position coordinates to geographic coordinates
        // Default values:
        // const double referenceLatitude = -33.9545;
        // const double referenceLongitude = 18.4563;
        // const double referenceAltitude = 0;

        // Rough estimation equirectangular projection method.
        double longitude = referenceLongitude + x / (cos(referenceLatitude * M_PI / 180) * 111319.9);
        double latitude = referenceLatitude + y / 111319.9;
        double altitudeAboveGround = altitude - referenceAltitude;

        // Get the motionpath element
        const DOMElement* motionPathElement = dynamic_cast<const DOMElement*>(element->getElementsByTagName(motionPathTag)->item(0));

        // Extract the interpolation attribute
        const XMLCh* interpolation = motionPathElement->getAttribute(interpolationAttr);

        // Determine if the interpolation is linear or exponential
        bool isLinearOrExponential = (XMLString::equals(interpolation, XMLString::transcode("linear")) || XMLString::equals(interpolation, XMLString::transcode("exponential")));

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

        // If the interpolation is linear or exponential, use the gx:Track element
        if (isLinearOrExponential) {
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

                // Write the time and coordinates to the gx:Track element
                kmlFile << "        <when>" << time << "</when>\n";
                kmlFile << "        <gx:coord>" << longitude << " " << latitude << " " << altitudeAboveGround << "</gx:coord>\n";
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

        if (isLinearOrExponential) {
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