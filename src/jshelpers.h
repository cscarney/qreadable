#ifndef JSHELPERS_H
#define JSHELPERS_H
#include <QFile>
#include <QDebug>
#include <QJSEngine>
#include <QJSValue>
#include <QJSValueIterator>

namespace JSHelpers {

inline void logError(const QJSValue &error)
{
    qWarning() << "unhandled js error"
             << error.property("message").toString()
             << error.property("lineNumber").toString()
             << error.property("stack").toString();
}

inline void logObject(const QJSValue &obj)
{
    QJSValueIterator it(obj);
    while (it.hasNext()) {
        it.next();
        qDebug() << it.name() << ": " << it.value().toString();
    }
}

inline QJSValue getProperty(QJSValue &jsObj, const QString &propertyName)
{
    QJSValue result = jsObj.property(propertyName);
    if (result.isError()) {
        logError(result);
        return QJSValue();
    }
    return result;
}

inline QJSValue callMember(QJSValue &jsObj, const QString &propertyName, const QJSValueList &args={})
{
    QJSValue func = getProperty(jsObj, propertyName);
    QJSValue result = func.callWithInstance(jsObj, args);
    if (result.isError()) {
        logError(result);
        return QJSValue();
    }
    return result;
}

inline QJSValue callConstructor(QJSValue &jsConstructor, const QJSValueList &args={})
{
    QJSValue result = jsConstructor.callAsConstructor(args);
    // result.prototype().setPrototype(result.prototype().property("__proto__"));
    if (result.isError()) {
        logError(result);
        return QJSValue();
    }
    return result;
}

inline QJSValue callMemberConstructor(QJSValue &jsObj, const QString &propertyName, const QJSValueList &args={})
{
    QJSValue constructor = getProperty(jsObj, propertyName);
    return callConstructor(constructor, args);
}

inline QJSValue evalFile(QJSEngine &engine, const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        return QJSValue();
    }
    QString code = file.readAll();
    QJSValue result = engine.evaluate(code);
    if (result.isError()) {
        logError(result);
        return QJSValue();
    }
    return result;
}

}

#endif // JSHELPERS_H
