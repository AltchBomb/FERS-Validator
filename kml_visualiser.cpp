#include <iostream>
#include <fstream>
#include <string>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>

using namespace std;
using namespace xercesc;

void processElement(const DOMElement* element, std::ofstream& kmlFile) {
    const XMLCh* platformTag = XMLString::transcode("platform");
    const XMLCh* receiverTag = XMLString::transcode("receiver");
    const XMLCh* transmitterTag = XMLString::transcode("transmitter");
    const XMLCh* targetTag = XMLString::transcode("target");
    const XMLCh* positionWaypointTag = XMLString::transcode("positionwaypoint");
    const XMLCh* xTag = XMLString::transcode("x");
    const XMLCh* yTag = XMLString::transcode("y");
    const XMLCh* altitudeTag = XMLString::transcode("altitude");

    // Check if the element is a platform
    if (XMLString::equals(element->getTagName(), platformTag)) {
        // Get the positionwaypoint element
        const DOMElement* positionWaypointElement = dynamic_cast<const DOMElement*>(
            element->getElementsByTagName(positionWaypointTag)->item(0));

        // Extract the position coordinates
        double x = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(xTag)->item(0)->getTextContent()));
        double y = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(yTag)->item(0)->getTextContent()));
        double altitude = std::stod(XMLString::transcode(positionWaypointElement->getElementsByTagName(altitudeTag)->item(0)->getTextContent()));

        // Convert the position coordinates to geographic coordinates
        const double referenceLatitude = -33.9545;
        const double referenceLongitude = 18.4563;
        const double referenceAltitude = 0;

        double longitude = referenceLongitude + x / (cos(referenceLatitude * M_PI / 180) * 111319.9);
        double latitude = referenceLatitude + y / 111319.9;
        double altitudeAboveGround = altitude - referenceAltitude;

        // Determine the type of placemark to use
        std::string placemarkType;
        if (element->getElementsByTagName(receiverTag)->getLength() > 0 || element->getElementsByTagName(transmitterTag)->getLength() > 0) {
            if (altitudeAboveGround > 0) {
                placemarkType = "floating";
            } else {
                placemarkType = "simple";
            }
        } else if (element->getElementsByTagName(targetTag)->getLength() > 0) {
            placemarkType = "extruded";
        }

        // Write the placemark data to the KML file
        kmlFile << "<Placemark>\n";
        kmlFile << "<name>" << XMLString::transcode(element->getAttribute(XMLString::transcode("name"))) << "</name>\n";
        kmlFile << "<description>" << XMLString::transcode(element->getAttribute(XMLString::transcode("description"))) << "</description>\n";
        kmlFile << "<styleUrl>#" << placemarkType << "</styleUrl>\n";
        kmlFile << "<Point>\n";
        kmlFile << "<coordinates>" << longitude << "," << latitude << "," << altitudeAboveGround << "</coordinates>\n";
        kmlFile << "</Point>\n";
        if (placemarkType == "extruded") {
            kmlFile << "<gx:altitudeMode>relativeToGround</gx:altitudeMode>\n";
            kmlFile << "<extrude>1</extrude>\n";
            kmlFile << "<height>" << altitudeAboveGround << "</height>\n";
        }
        kmlFile << "</Placemark>\n";
    }
}

void traverseDOMNode(const DOMNode* node, std::ofstream& kmlFile) {
    if (node->getNodeType() == DOMNode::ELEMENT_NODE) {
        const DOMElement* element = dynamic_cast<const DOMElement*>(node);
        processElement(element, kmlFile);
    }

    for (DOMNode* child = node->getFirstChild(); child != nullptr; child = child->getNextSibling()) {
        traverseDOMNode(child, kmlFile);
    }
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input XML file> <output KML file>" << std::endl;
        return 1;
    }

     // Update file_path with command line argument
    string file_path = argv[1];
    // Setting mode to evironment variable from command line.
    string output_file = argv[2];

    try {
        XMLPlatformUtils::Initialize();
        //std::cout << mode << std::endl;
        XercesDOMParser parser;
        parser.setValidationScheme(XercesDOMParser::Val_Never);
        parser.setDoNamespaces(false);
        parser.setDoSchema(false);
        parser.setLoadExternalDTD(false);

        //parser.parse("/Users/michaelaltshuler/Documents/5th Year/EEE4022F:Thesis/FERS Features/FERS Validator/FERSXML-example/SingleSimDualTargetTest.fersxml");

        // Use file_path from command line argument
        parser.parse(file_path.c_str()); 
        std::cout << "parsed" << std::endl;
        DOMDocument* document = parser.getDocument();
        std::cout << "document created: " << document << std::endl;
        DOMElement* rootElement = document->getDocumentElement();
        std::cout << "root address found" << std::endl;

        std::ofstream kmlFile(output_file.c_str());
        if (!kmlFile.is_open()) {
            std::cerr << "Error opening output KML file" << std::endl;
            XMLPlatformUtils::Terminate();
            return 1;
        }

        // Write the KML header
        kmlFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        kmlFile << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";
        kmlFile << "<Document>\n";

        traverseDOMNode(rootElement, kmlFile);

        // Write the KML footer
        kmlFile << "</Document>\n";
        kmlFile << "</kml>\n";

        kmlFile.close();
        XMLPlatformUtils::Terminate();
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

    return 0;
}