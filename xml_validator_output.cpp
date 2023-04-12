#include <iostream>
#include <string>
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

void outputElement(DOMElement* element, int indent = 0) {
    DOMNodeList* children = element->getChildNodes();
    int numChildren = children->getLength();
    for (int i = 0; i < numChildren; i++) {
        DOMNode* childNode = children->item(i);
        if (childNode->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement* childElement = dynamic_cast<DOMElement*>(childNode);
            string elementName = XMLString::transcode(childElement->getNodeName());
            cout << string(indent, ' ') << elementName << ":";

            DOMNamedNodeMap* attributes = childElement->getAttributes();
            int numAttributes = attributes->getLength();
            if (numAttributes > 0) {
                cout << " ";
                for (int j = 0; j < numAttributes; j++) {
                    DOMAttr* attribute = dynamic_cast<DOMAttr*>(attributes->item(j));
                    string attributeName = XMLString::transcode(attribute->getName());
                    string attributeValue = XMLString::transcode(attribute->getValue());
                    cout << attributeName << "=" << attributeValue;
                    if (j < numAttributes - 1) {
                        cout << "; ";
                    }
                }
            }
            cout << endl;

            outputElement(childElement, indent + 2);
        }
        else if (childNode->getNodeType() == DOMNode::TEXT_NODE) {
            DOMText* textNode = dynamic_cast<DOMText*>(childNode);
            string textValue = XMLString::transcode(textNode->getData());
            cout << string(indent, ' ') << textValue << endl;
        }
    }
}

int main() {
    try {
        XMLPlatformUtils::Initialize();

        XercesDOMParser parser;
        parser.setValidationScheme(XercesDOMParser::Val_Never);
        parser.setDoNamespaces(false);
        parser.setDoSchema(false);
        parser.setLoadExternalDTD(false);

        parser.parse("/Users/michaelaltshuler/Documents/5th Year/EEE4022F:Thesis/FERS Features/FERS Validator/FERSXML-example/mono.xml");

        DOMDocument* document = parser.getDocument();
        DOMElement* rootElement = document->getDocumentElement();
        outputElement(rootElement);

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
