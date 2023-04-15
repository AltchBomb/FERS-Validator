// Part of FERS input Validator Function
// Outputs FERSXML contents to console in readbale format.
// Script written by Michael Altshuler 
// University of Cape Town: ALTMIC003

#include <iostream>
#include <string>

// Necessary Xerces-C++ header files for parsing the 
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax/SAXException.hpp> 

using namespace xercesc;
using namespace std;

// Setting default values if not specified in FERSXML file
// Default values for rsparameters.cpp
double default_c = 299792458.0;
double default_starttime = 0.0;
double default_endtime = 0.0;
double default_interprate = 1000.0;
double default_rate = 0.0;
double default_adc_bits = 0.0;
double default_oversample = 1.0;
// Extract export element values within inputted FERSXML file or use default values
bool binary = true;  // Default value for binary parameter
bool csv = false;   // Default value for csv parameter
bool xml = false;   // Default value for xml parameter

bool binarySpecified = false;  // Flag to indicate if binary value was specified
bool csvSpecified = false;     // Flag to indicate if csv value was specified
bool xmlSpecified = false;     // Flag to indicate if xml value was specified

// Default values for rstiming.cpp
double default_freq_offset = 0.0;
double default_phase_offset = 0.0;
double default_frequency = 0.0;

// Recursive function that outputs the content of a DOM element
void outputElement(DOMElement* element, int indent = 0) {
    DOMNodeList* children = element->getChildNodes();
    int numChildren = children->getLength();
    for (int i = 0; i < numChildren; i++) {
        DOMNode* childNode = children->item(i);
        if (childNode->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement* childElement = dynamic_cast<DOMElement*>(childNode);
            string elementName = XMLString::transcode(childElement->getNodeName());
            // Outputs element name with its identification
            cout << string(indent, ' ') << elementName << ":";

            DOMNamedNodeMap* attributes = childElement->getAttributes();
            int numAttributes = attributes->getLength();
            // If statement outputs element attributes if there are any
            if (numAttributes > 0) {
                cout << " ";
                for (int j = 0; j < numAttributes; j++) {
                    DOMAttr* attribute = dynamic_cast<DOMAttr*>(attributes->item(j));
                    string attributeName = XMLString::transcode(attribute->getName());
                    string attributeValue = XMLString::transcode(attribute->getValue());
                    // Outputs the attributes name and value
                    cout << attributeName << "=" << attributeValue;
                    if (j < numAttributes - 1) {
                        cout << "; ";
                    }
                }
            }
            cout << endl;

            // Recursively process child elements
            outputElement(childElement, indent + 2);
        }
        // Output text content of the element
        else if (childNode->getNodeType() == DOMNode::TEXT_NODE) {
            DOMText* textNode = dynamic_cast<DOMText*>(childNode);
            string textValue = XMLString::transcode(textNode->getData());
            // Output the text content with identification
            cout << string(indent, ' ') << textValue << endl;
        }
    }
}

int main(int argc, char* argv[]) {

    // Update file_path with command line argument
    string file_path = argv[1]; 

    try {
        XMLPlatformUtils::Initialize();

        XercesDOMParser parser;
        parser.setValidationScheme(XercesDOMParser::Val_Never);
        parser.setDoNamespaces(false);
        parser.setDoSchema(false);
        parser.setLoadExternalDTD(false);

        //parser.parse("/Users/michaelaltshuler/Documents/5th Year/EEE4022F:Thesis/FERS Features/FERS Validator/FERSXML-example/SingleSimDualTargetTest.fersxml");

        // Use file_path from command line argument
        parser.parse(file_path.c_str()); 

        DOMDocument* document = parser.getDocument();
        DOMElement* rootElement = document->getDocumentElement();
        outputElement(rootElement);

        // Extract set element values within inputted FERSXML file or use default values

        // Assigning default value to c:
        double c = default_c;

        DOMNodeList *cNodes = rootElement->getElementsByTagName(XMLString::transcode("c"));
        if (cNodes->getLength() > 0) {
            DOMNode *cNode = cNodes->item(0);
            if (cNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMElement *cElement = static_cast<DOMElement*>(cNode);
                const XMLCh *cValue = cElement->getTextContent();
                if (cValue) {
                    char *cStr = XMLString::transcode(cValue);
                    c = std::stod(cStr);
                    XMLString::release(&cStr);
                }
            }
        } else {
            c = default_c;
        }

        // Assigning default value to starttime:
        double starttime = default_starttime;

        DOMNodeList *starttimeNodes = rootElement->getElementsByTagName(XMLString::transcode("starttime"));
        if (starttimeNodes->getLength() > 0) {
            DOMNode *starttimeNode = starttimeNodes->item(0);
            if (starttimeNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMElement *starttimeElement = static_cast<DOMElement*>(starttimeNode);
                const XMLCh *starttimeValue = starttimeElement->getTextContent();
                if (starttimeValue) {
                    char *starttimeStr = XMLString::transcode(starttimeValue);
                    starttime = std::stod(starttimeStr);
                    XMLString::release(&starttimeStr);
                }
            }
        } else {
            starttime = default_starttime;
        }

        // Assigning default value to endtime
        double endtime = default_endtime;

        DOMNodeList *endtimeNodes = rootElement->getElementsByTagName(XMLString::transcode("endtime"));
        if (endtimeNodes->getLength() > 0) {
            DOMNode *endtimeNode = endtimeNodes->item(0);
            if (endtimeNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMElement *endtimeElement = static_cast<DOMElement*>(endtimeNode);
                const XMLCh *endtimeValue = endtimeElement->getTextContent();
                if (endtimeValue) {
                    char *endtimeStr = XMLString::transcode(endtimeValue);
                    endtime = std::stod(endtimeStr);
                    XMLString::release(&endtimeStr);
                }
            }
        } else {
            endtime = default_endtime;
        }

        // Assigning the default value to interprate
        double interprate = default_interprate;

        DOMNodeList *interprateNodes = rootElement->getElementsByTagName(XMLString::transcode("interprate"));
        if (interprateNodes->getLength() > 0) {
            DOMNode *interprateNode = interprateNodes->item(0);
            if (interprateNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMElement *interprateElement = static_cast<DOMElement*>(interprateNode);
                const XMLCh *interprateValue = interprateElement->getTextContent();
                if (interprateValue) {
                    char *interprateStr = XMLString::transcode(interprateValue);
                    interprate = std::stod(interprateStr);
                    XMLString::release(&interprateStr);
                }
            }
        } else {
            interprate = default_interprate;
        }

        // Assigning the defaut value to rate
        double rate = default_rate;

        DOMNodeList *rateNodes = rootElement->getElementsByTagName(XMLString::transcode("rate"));
        if (rateNodes->getLength() > 0) {
            DOMNode *rateNode = rateNodes->item(0);
            if (rateNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMElement *rateElement = static_cast<DOMElement*>(rateNode);
                const XMLCh *rateValue = rateElement->getTextContent();
                if (rateValue) {
                    char *rateStr = XMLString::transcode(rateValue);
                    rate = std::stod(rateStr);
                    XMLString::release(&rateStr);
                }
            }
        } else {
            rate = default_rate;
        }

        // Assigning default value to adc_bits:
        double adc_bits = default_adc_bits;

        DOMNodeList *adc_bitsNodes = rootElement->getElementsByTagName(XMLString::transcode("adc_bits"));
        if (adc_bitsNodes->getLength() > 0) {
            DOMNode *adc_bitsNode = adc_bitsNodes->item(0);
            if (adc_bitsNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMElement *adc_bitsElement = static_cast<DOMElement*>(adc_bitsNode);
                const XMLCh *adc_bitsValue = adc_bitsElement->getTextContent();
                if (adc_bitsValue) {
                    char *adc_bitsStr = XMLString::transcode(adc_bitsValue);
                    adc_bits = std::stod(adc_bitsStr);
                    XMLString::release(&adc_bitsStr);
                }
            }
        } else {
            adc_bits = default_adc_bits;
        }

        // Assigning default value to oversample:
        double oversample = default_oversample;

        DOMNodeList *oversampleNodes = rootElement->getElementsByTagName(XMLString::transcode("oversample"));
        if (oversampleNodes->getLength() > 0) {
            DOMNode *oversampleNode = oversampleNodes->item(0);
            if (oversampleNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMElement *oversampleElement = static_cast<DOMElement*>(oversampleNode);
                const XMLCh *oversampleValue = oversampleElement->getTextContent();
                if (oversampleValue) {
                    char *oversampleStr = XMLString::transcode(oversampleValue);
                    adc_bits = std::stod(oversampleStr);
                    XMLString::release(&oversampleStr);
                }
            }
        } else {
            oversample = default_oversample;
        }

        // Assigning default valies to the 'export' paramters
        // xml = false; csv = false; binary = true
        DOMNodeList *exportNodes = rootElement->getElementsByTagName(XMLString::transcode("export"));
        if (exportNodes->getLength() > 0) {
            DOMNode *exportNode = exportNodes->item(0);
            if (exportNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMElement *exportElement = static_cast<DOMElement*>(exportNode);

                // Get binary attribute value or use default value
                const XMLCh* binaryValue = exportElement->getAttribute(XMLString::transcode("binary"));
                if (binaryValue) {
                    char* binaryStr = XMLString::transcode(binaryValue);
                    std::string binaryAttr = binaryStr;
                    if (binaryAttr == "false") {
                        binary = false;
                    }
                    XMLString::release(&binaryStr);
                }

                // Get csv attribute value or use default value
                const XMLCh* csvValue = exportElement->getAttribute(XMLString::transcode("csv"));
                if (csvValue) {
                    char* csvStr = XMLString::transcode(csvValue);
                    std::string csvAttr = csvStr;
                    if (csvAttr == "true") {
                        csv = true;
                    }
                    XMLString::release(&csvStr);
                }

                // Get xml attribute value or use default value
                const XMLCh* xmlValue = exportElement->getAttribute(XMLString::transcode("xml"));
                if (xmlValue) {
                    char* xmlStr = XMLString::transcode(xmlValue);
                    std::string xmlAttr = xmlStr;
                    if (xmlAttr == "true") {
                        xml = true;
                    }
                    XMLString::release(&xmlStr);
                }
            }
        }

        // Assigning default value to freq_offset:
        double freq_offset = default_freq_offset;

        DOMNodeList *freq_offsetNodes = rootElement->getElementsByTagName(XMLString::transcode("freq_offset"));
        if (freq_offsetNodes->getLength() > 0) {
            DOMNode *freq_offsetNode = freq_offsetNodes->item(0);
            if (freq_offsetNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMElement *freq_offsetElement = static_cast<DOMElement*>(freq_offsetNode);
                const XMLCh *freq_offsetValue = freq_offsetElement->getTextContent();
                if (freq_offsetValue) {
                    char *freq_offsetStr = XMLString::transcode(freq_offsetValue);
                    freq_offset = std::stod(freq_offsetStr);
                    XMLString::release(&freq_offsetStr);
                }
            }
        } else {
            freq_offset = default_freq_offset;
        }

        // Assigning default value to freq_offset:
        double phase_offset = default_phase_offset;

        DOMNodeList *phase_offsetNodes = rootElement->getElementsByTagName(XMLString::transcode("phase_offset"));
        if (phase_offsetNodes->getLength() > 0) {
            DOMNode *phase_offsetNode = phase_offsetNodes->item(0);
            if (phase_offsetNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMElement *phase_offsetElement = static_cast<DOMElement*>(phase_offsetNode);
                const XMLCh *phase_offsetValue = phase_offsetElement->getTextContent();
                if (phase_offsetValue) {
                    char *phase_offsetStr = XMLString::transcode(phase_offsetValue);
                    phase_offset = std::stod(phase_offsetStr);
                    XMLString::release(&phase_offsetStr);
                }
            }
        } else {
            phase_offset = default_phase_offset;
        }

        // Assigning default value to frequency:
        double frequency = default_frequency;

        DOMNodeList *frequencyNodes = rootElement->getElementsByTagName(XMLString::transcode("frequency"));
        if (frequencyNodes->getLength() > 0) {
            DOMNode *frequencyNode = frequencyNodes->item(0);
            if (frequencyNode->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMElement *frequencyElement = static_cast<DOMElement*>(frequencyNode);
                const XMLCh *frequencyValue = frequencyElement->getTextContent();
                if (frequencyValue) {
                    char *frequencyStr = XMLString::transcode(frequencyValue);
                    frequency = std::stod(frequencyStr);
                    XMLString::release(&frequencyStr);
                }
            }
        } else {
            frequency = default_frequency;
        }

        // Print element values to console
        std::cout << "The following default values have been applied to the FERS simulation, unless otherwise specified:" << std::endl;

        // <parameters> defaults:
        std::cout << "<parameters> defaults:" << std::endl;
        // Outputting default value for c (speed of light)
        if (c == default_c && cNodes->getLength() == 0) {
            std::cout << "  c = " << c << " m.s^-1" <<std::endl;
        }
        // Outputting default value for starttime
        if (starttime == default_starttime && starttimeNodes->getLength() == 0) {
            std::cout << "  starttime = " << starttime << " Seconds" << std::endl;
        }
        // Outputting default value for endtime
        if (endtime == default_endtime && endtimeNodes->getLength() == 0) {
            std::cout << "  endtime = " << endtime << " Seconds" << std::endl;
        }
        // Outputting default value for interprate
        if (interprate == default_interprate && interprateNodes->getLength() == 0) {
            std::cout << "  interprate = " << interprate << " Per second" << std::endl;
        }
        // Outputting default value for rate
        if (rate == default_rate && rateNodes->getLength() == 0) {
            std::cout << "  rate = " << rate << std::endl;
        }
        // Outputting default values for export parameters
        if (!binarySpecified || !csvSpecified || !xmlSpecified) {
            std::cout << "  export default values:" << std::endl;
            if (!binarySpecified) {
                std::cout << "      binary: " << (binary ? "true" : "false") << std::endl;
            }
            if (!csvSpecified) {
                std::cout << "      csv: " << (csv ? "true" : "false") << std::endl;
            }
            if (!xmlSpecified) {
                std::cout << "      xml: " << (xml ? "true" : "false") << std::endl;
            }
        }
        // Outputting default value for adc_bits
        if (adc_bits == default_adc_bits && adc_bitsNodes->getLength() == 0) {
            std::cout << "  adc_bits = " << adc_bits << std::endl;
        }
        // Outputting default value for oversample
        if (oversample == default_oversample && oversampleNodes->getLength() == 0) {
            std::cout << "  oversample = " << oversample << std::endl;
        }
        std::cout << "" << std::endl;

        // <timing> defaults:
        std::cout << "<timing> defaults:" << std::endl;
        // Assigning default value for freq_offset
        if (freq_offset == default_freq_offset && freq_offsetNodes->getLength() == 0) {
            std::cout << "  freq_offset = " << freq_offset << " Hz" << std::endl;
        }
        // Assigning value for phase_offset
        if (phase_offset == default_phase_offset && phase_offsetNodes->getLength() == 0) {
            std::cout << "  phase_offset = " << phase_offset << std::endl;
        }
        // Assigning vlaue for frequency
        if (frequency == default_frequency && frequencyNodes->getLength() == 0) {
            std::cout << "  frequency = " << frequency << " Hz" << std::endl;
        }
        

        // Free up memory to prevent segmentation fault
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
