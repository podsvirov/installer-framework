/**************************************************************************
**
** This file is part of Qt SDK**
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
**
** Contact:  Nokia Corporation qt-info@nokia.com**
**
** No Commercial Usage
**
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception version
** 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you are unsure which license is appropriate for your use, please contact
** (qt-info@nokia.com).
**
**************************************************************************/
#include "updatesettings.h"

#include "common/errors.h"
#include "common/installersettings.h"
#include "common/repository.h"

#include <QtCore/QDateTime>
#include <QtCore/QSettings>
#include <QtCore/QstringList>

using namespace QInstaller;

class UpdateSettings::Private
{
public:
    Private(UpdateSettings* qq)
        : q(qq)
    {
    }

private:
    UpdateSettings *const q;

public:
    QSettings& settings()
    {
        return externalSettings ? *externalSettings : internalSettings;
    }

    const QSettings& settings() const
    {
        return externalSettings ? *externalSettings : internalSettings;
    }

    QSettings internalSettings;
    static QSettings* externalSettings;
};

QSettings* UpdateSettings::Private::externalSettings = 0;


// -- UpdateSettings

UpdateSettings::UpdateSettings()
    : d(new Private(this))
{
    d->settings().sync();
}

UpdateSettings::~UpdateSettings()
{
    d->settings().sync();
}

/* static */
void UpdateSettings::setSettingsSource(QSettings *settings)
{
    Private::externalSettings = settings;
}

int UpdateSettings::updateInterval() const
{
    return d->settings().value(QLatin1String("updatesettings/interval"), static_cast<int> (Weekly)).toInt();
}

void UpdateSettings::setUpdateInterval(int seconds)
{
    d->settings().setValue(QLatin1String("updatesettings/interval"), seconds);
}

QString UpdateSettings::lastResult() const
{
    return d->settings().value(QLatin1String("updatesettings/lastresult")).toString();
}

void UpdateSettings::setLastResult(const QString &lastResult)
{
    d->settings().setValue(QLatin1String("updatesettings/lastresult"), lastResult);
}

QDateTime UpdateSettings::lastCheck() const
{
    return d->settings().value(QLatin1String("updatesettings/lastcheck")).toDateTime();
}

void UpdateSettings::setLastCheck(const QDateTime &lastCheck)
{
    d->settings().setValue(QLatin1String("updatesettings/lastcheck"), lastCheck);
}

bool UpdateSettings::checkOnlyImportantUpdates() const
{
    return d->settings().value(QLatin1String("updatesettings/onlyimportant"), false).toBool();
}

void UpdateSettings::setCheckOnlyImportantUpdates(bool checkOnlyImportantUpdates)
{
    d->settings().setValue(QLatin1String("updatesettings/onlyimportant"), checkOnlyImportantUpdates);
}

QList<Repository> UpdateSettings::repositories() const
{
    QSettings &settings = *(const_cast<QSettings*> (&d->settings()));
    const int count = settings.beginReadArray(QLatin1String("updatesettings/repositories"));

    QList<Repository> result;
    for (int i = 0; i < count; ++i) {
        Repository rep;
        settings.setArrayIndex(i);
        rep.setUrl(d->settings().value(QLatin1String("url")).toUrl());
        result.append(rep);
    }
    settings.endArray();

    try {
        if(result.isEmpty()) {
            result = Settings::fromFileAndPrefix(QLatin1String(":/metadata/installer-config/config.xml"),
                QLatin1String(":/metadata/installer-config/")).repositories();
        }
    } catch (const Error &e) {
        qDebug("Could not parse config: %s", qPrintable(e.message()));
    }
    return result;
}

void UpdateSettings::setRepositories(const QList<Repository> &repositories)
{
    d->settings().beginWriteArray(QLatin1String("updatesettings/repositories"));
    for (int i = 0; i < repositories.count(); ++i) {
        const Repository &rep = repositories.at(i);
        d->settings().setArrayIndex(i);
        d->settings().setValue(QLatin1String("url"), rep.url());
    }
    d->settings().endArray();
}
