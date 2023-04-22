// FERS input Validator Function sub-system
// Outputs KML GIS readable file.
// Script written by Michael Altshuler 
// University of Cape Town: ALTMIC003

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
        const double referenceLatitude = -33.9545;
        const double referenceLongitude = 18.4563;
        const double referenceAltitude = 0;

        double longitude = referenceLongitude + x / (cos(referenceLatitude * M_PI / 180) * 111319.9);
        double latitude = referenceLatitude + y / 111319.9;
        double altitudeAboveGround = altitude - referenceAltitude;

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
        XercesDOMParser parser;

        // Error handler configuration
        ErrorHandler* errorHandler = (ErrorHandler*) new HandlerBase();
        parser.setErrorHandler(errorHandler); 

        parser.setValidationScheme(XercesDOMParser::Val_Never);
        parser.setDoNamespaces(false);
        parser.setDoSchema(false);
        parser.setLoadExternalDTD(false);

        //parser.parse("/Users/michaelaltshuler/Documents/5th Year/EEE4022F:Thesis/FERS Features/FERS Validator/FERSXML-example/SingleSimDualTargetTest.fersxml");

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
        kmlFile << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";
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
        kmlFile << "    <longitude>18.4612</longitude>\n";
        kmlFile << "    <latitude>-33.9545</latitude>\n";
        kmlFile << "    <altitude>0</altitude>\n";
        kmlFile << "    <heading>-148.4122922628044</heading>\n";
        kmlFile << "    <tilt>40.5575073395506</tilt>\n";
        kmlFile << "    <range>2500</range>\n";
        kmlFile << "  </LookAt>\n";

        // Traverse DOMNode and output extracted FERSXML data:
        traverseDOMNode(rootElement, kmlFile);

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