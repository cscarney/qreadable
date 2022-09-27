#ifndef TST_DOMSUPPORT_JSCODE_H
#define TST_DOMSUPPORT_JSCODE_H

namespace {
constexpr const char *kBaseTestCase = R"!!!(<html><body><p>Some text and <a class="someclass" href="#">a link</a></p><div id="foo">With a <script>With &lt; fancy " characters in it because</script> that is fun.<span>And another node to make it harder</span></div><form><input type="text"/><input type="number"/>Here\'s a form</form></body></html>)!!!";

constexpr const char *kTestPrelude = R"!!!(
  this.verify = test_support.verify;

  this.nodeAssert = function(actual, expected) {
    verify(actual === expected);
  }
)!!!";

constexpr const char *kTestBaseDocHierarchy = R"!!!(
    verify(baseDoc.childNodes.length===1);
    verify(baseDoc.getElementsByTagName("*").length == 11);
    var foo = baseDoc.getElementById("foo");
    verify(foo.parentNode.localName==="body");
    nodeAssert(baseDoc.body, foo.parentNode);
    nodeAssert(baseDoc.body.parentNode, baseDoc.documentElement);

    var generatedHTML = baseDoc.getElementsByTagName("p")[0].innerHTML;
    verify(generatedHTML==='Some text and <a class="someclass" href="#">a link</a>');
)!!!";

constexpr const char *kTestPrevNextConnections = R"!!!(
    var foo = baseDoc.getElementById("foo");

    nodeAssert(foo.previousSibling.nextSibling, foo);
    nodeAssert(foo.nextSibling.previousSibling, foo);
    nodeAssert(foo.nextSibling, foo.nextElementSibling);
    nodeAssert(foo.previousSibling, foo.previousElementSibling);

    var beforeFoo = foo.previousSibling;
    var afterFoo = foo.nextSibling;

    nodeAssert(baseDoc.body.firstChild, beforeFoo);
)!!!";

constexpr const char *kTestRemoveAppend = R"!!!(
    var foo = baseDoc.getElementById("foo");
    var beforeFoo = foo.previousSibling;
    var afterFoo = foo.nextSibling;

    var bodyBefore = baseDoc.body.childNodes.length;
    var removedFoo = foo.parentNode.removeChild(foo);
    nodeAssert(foo, removedFoo);
    nodeAssert(foo.parentNode, null);
    nodeAssert(foo.previousSibling, null);
    nodeAssert(foo.nextSibling, null);
    nodeAssert(foo.previousElementSibling, null);
    nodeAssert(foo.nextElementSibling, null);

    verify(beforeFoo.localName==="p");
    nodeAssert(beforeFoo.nextSibling, afterFoo);
    nodeAssert(afterFoo.previousSibling, beforeFoo);
    nodeAssert(beforeFoo.nextElementSibling, afterFoo);
    nodeAssert(afterFoo.previousElementSibling, beforeFoo);

    verify(baseDoc.body.childNodes.length===bodyBefore-1);

    var oldLast = baseDoc.body.lastChild;
    baseDoc.body.appendChild(foo);

    verify(baseDoc.body.childNodes.length===bodyBefore);
    nodeAssert(oldLast.nextSibling, foo);
    nodeAssert(foo.previousSibling, oldLast);
    nodeAssert(afterFoo.nextElementSibling, foo);
    nodeAssert(foo.previousElementSibling, afterFoo);
)!!!";

constexpr const char *kTestReplaceChild = R"!!!(
    var parent = baseDoc.getElementsByTagName("div")[0];
    var p = baseDoc.createElement("p");
    p.setAttribute("id", "my-replaced-kid");
    var childCount = parent.childNodes.length;
    var childElCount = parent.children.length;
    for (var i = 0; i < parent.childNodes.length; i++) {
      var replacedNode = parent.childNodes[i];
      var replacedAnElement = replacedNode.nodeType === replacedNode.ELEMENT_NODE;
      var oldNext = replacedNode.nextSibling;
      var oldNextEl = replacedNode.nextElementSibling;
      var oldPrev = replacedNode.previousSibling;
      var oldPrevEl = replacedNode.previousElementSibling;

      parent.replaceChild(p, replacedNode);

      // Check siblings and parents on both nodes were set:
      nodeAssert(p.nextSibling, oldNext);
      nodeAssert(p.previousSibling, oldPrev);
      nodeAssert(p.parentNode, parent);

      nodeAssert(replacedNode.parentNode, null);
      nodeAssert(replacedNode.nextSibling, null);
      nodeAssert(replacedNode.previousSibling, null);
      // if the old node was an element, element siblings should now be null
      if (replacedAnElement) {
        nodeAssert(replacedNode.nextElementSibling, null);
        nodeAssert(replacedNode.previousElementSibling, null);
      }

      // Check the siblings were updated
      if (oldNext)
        nodeAssert(oldNext.previousSibling, p);
      if (oldPrev)
        nodeAssert(oldPrev.nextSibling, p);

      // check the array was updated
      nodeAssert(parent.childNodes[i], p);

      // Now check element properties/lists:
      var kidElementIndex = parent.children.indexOf(p);
      // should be in the list:
      verify(kidElementIndex!=-1);

      if (kidElementIndex > 0) {
        nodeAssert(parent.children[kidElementIndex - 1], p.previousElementSibling);
        nodeAssert(p.previousElementSibling.nextElementSibling, p);
      } else {
        nodeAssert(p.previousElementSibling, null);
      }
      if (kidElementIndex < parent.children.length - 1) {
        nodeAssert(parent.children[kidElementIndex + 1], p.nextElementSibling);
        nodeAssert(p.nextElementSibling.previousElementSibling, p);
      } else {
        nodeAssert(p.nextElementSibling, null);
      }

      if (replacedAnElement) {
        nodeAssert(oldNextEl, p.nextElementSibling);
        nodeAssert(oldPrevEl, p.previousElementSibling);
      }

      verify(parent.childNodes.length===childCount);
      verify(parent.children.length===replacedAnElement ? childElCount : childElCount + 1);

      parent.replaceChild(replacedNode, p);

      nodeAssert(oldNext, replacedNode.nextSibling);
      nodeAssert(oldNextEl, replacedNode.nextElementSibling);
      nodeAssert(oldPrev, replacedNode.previousSibling);
      nodeAssert(oldPrevEl, replacedNode.previousElementSibling);
      if (replacedNode.nextSibling)
        nodeAssert(replacedNode.nextSibling.previousSibling, replacedNode);
      if (replacedNode.previousSibling)
        nodeAssert(replacedNode.previousSibling.nextSibling, replacedNode);
      if (replacedAnElement) {
        if (replacedNode.previousElementSibling)
          nodeAssert(replacedNode.previousElementSibling.nextElementSibling, replacedNode);
        if (replacedNode.nextElementSibling)
          nodeAssert(replacedNode.nextElementSibling.previousElementSibling, replacedNode);
      }
    }
)!!!";

constexpr const char *kEscapeTestCase = "<p>Hello, everyone &amp; all their friends, &lt;this&gt; is a &quot; test with &apos; quotes.</p>";

constexpr const char *kTestHTMLEscapes = R"!!!(
    var p = doc.getElementsByTagName("p")[0];
    var txtNode = p.firstChild;
    var normalizedHTML = baseStr.replace("&apos;", "'");

    // before modification, we should be using the original HTML from the doc
    verify("<p>" + p.innerHTML + "</p>"===baseStr);
    verify("<p>" + txtNode.innerHTML + "</p>"===baseStr);

    verify(p.textContent==="Hello, everyone & all their friends, <this> is a \" test with ' quotes.");
    verify(txtNode.textContent==="Hello, everyone & all their friends, <this> is a \" test with ' quotes.");
    txtNode.textContent = txtNode.textContent + " ";
    txtNode.textContent = txtNode.textContent.trim();

    // after modification, we should be using generated HTML
    verify("<p>" + txtNode.innerHTML + "</p>"===normalizedHTML);
    verify("<p>" + p.innerHTML + "</p>"===normalizedHTML);
)!!!";

constexpr const char *kNamespaceTestCase = "<a0:html><a0:body><a0:DIV><a0:svG><a0:clippath/></a0:svG></a0:DIV></a0:body></a0:html>";

constexpr const char *kTestNamespaces = R"!!!(
    var div = doc.getElementsByTagName("div")[0];
    verify(div.tagName==="DIV");
    verify(div.localName==="div");
    verify(div.firstChild.tagName==="SVG");
    verify(div.firstChild.localName==="svg");
    verify(div.firstChild.firstChild.tagName==="CLIPPATH");
    verify(div.firstChild.firstChild.localName==="clippath");
    verify(doc.documentElement===doc.firstChild);
)!!!";

}

#endif // TST_DOMSUPPORT_JSCODE_H
