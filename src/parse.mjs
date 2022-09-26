import { parseHTML } from "./linkedom.mjs"
import { Readability } from "./Readability.mjs"

function parse(text)
{
    let doc = parseHTML(text).document;
    let r = new Readability(doc);
    r.parse();
    return doc.innerHTML;
}

export { parse }
