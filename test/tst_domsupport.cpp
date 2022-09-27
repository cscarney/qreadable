#include <QtTest>
#include <QQmlEngine>
#include <memory>

#include "jshelpers.h"
#include "dombuilder.h"

// raw string literals confuse moc, so put them in a header file
#include "test/tst_domsupport_jscode.h"

using namespace QReadable;

class TestSupport : public QObject {
    Q_OBJECT
    QPointer<QJSEngine> m_engine;
    bool m_failure{false};
public:
    explicit TestSupport(QJSEngine *engine)
        : m_engine(engine)
    {}

    Q_INVOKABLE void verify(bool cond)
    {
        if (!cond) {
            JSHelpers::logError(m_engine->evaluate("new Error()"));
            m_failure = true;
        }
    }

    bool didSucceed() const {
        return !m_failure;
    }
};

static constexpr const char *kTestUrl = "http://fakehost/";

static void baseDoc(QJSEngine *engine)
{
    DomBuilder builder(kBaseTestCase);
    DomSupport::Document *doc = builder.buildDocument(QUrl(kTestUrl));
    engine->globalObject().setProperty("baseDoc", engine->newQObject(doc));
}

class testDomSupport : public QObject
{
    Q_OBJECT
    std::unique_ptr<QJSEngine> m_engine;
    QPointer<TestSupport> m_testSupport;

private slots:
    void initTestCase()
    {
    }

    void init()
    {
        m_engine = std::make_unique<QJSEngine>();
        m_engine->installExtensions(QJSEngine::ConsoleExtension);
        m_testSupport = new TestSupport(m_engine.get());
        QJSValue globalObject = m_engine->globalObject();
        QJSValue testSupport = m_engine->newQObject(m_testSupport);
        globalObject.setProperty("test_support",testSupport);
        QJSValue result = m_engine->evaluate(kTestPrelude);
        if (result.isError()) {
            JSHelpers::logError(result);
            assert(false);
        }
    }

    void cleanup()
    {
        m_engine.reset();
    }

    void testBaseDocHierarchy()
    {
        baseDoc(m_engine.get());
        QJSValue result = m_engine->evaluate(kTestBaseDocHierarchy);
        QVERIFY(!result.isError());
        QVERIFY(m_testSupport->didSucceed());
    }

    void testPrevNextConnections()
    {
        baseDoc(m_engine.get());
        QJSValue result = m_engine->evaluate(kTestPrevNextConnections);
        QVERIFY(!result.isError());
        QVERIFY(m_testSupport->didSucceed());
    }

    void testRemoveAppend()
    {
        baseDoc(m_engine.get());
        QJSValue result = m_engine->evaluate(kTestRemoveAppend);
        QVERIFY(!result.isError());
        QVERIFY(m_testSupport->didSucceed());
    }

    void testReplaceChild()
    {
        baseDoc(m_engine.get());
        QJSValue result = m_engine->evaluate(kTestReplaceChild);
        QVERIFY(!result.isError());
        QVERIFY(m_testSupport->didSucceed());
    }

    void testHTMLEscapes()
    {
        DomBuilder builder(kEscapeTestCase);
        DomSupport::Document *doc = builder.buildDocument(QUrl(kTestUrl));
        m_engine->globalObject().setProperty("baseStr", QString(kEscapeTestCase));
        m_engine->globalObject().setProperty("doc", m_engine->newQObject(doc));
        QJSValue result = m_engine->evaluate(kTestHTMLEscapes);
        QVERIFY(!result.isError());
        QVERIFY(m_testSupport->didSucceed());
    }

    void testNamespaces()
    {
        DomBuilder builder(kNamespaceTestCase);
        DomSupport::Document *doc = builder.buildDocument(QUrl(kTestUrl));
        m_engine->globalObject().setProperty("doc", m_engine->newQObject(doc));
        QJSValue result = m_engine->evaluate(kTestNamespaces);
        if (result.isError()) {
            JSHelpers::logError(result);
        }
        QVERIFY(!result.isError());
        QVERIFY(m_testSupport->didSucceed());
    }
};

QTEST_MAIN(testDomSupport)
#include "tst_domsupport.moc"

