/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*
 * author: 2017, Riccardo Maria Bianchi @ CERN -
 * <riccardo.maria.bianchi@cern.ch> major updates:
 *  - Aug 2018, R.M.Bianchi
 *  - Jun 2020, R.M.Bianchi
 *  - Aug 2020, R.M.Bianchi - Added support to publish lists of FullPhysVol and
 *                            AlignableTransform nodes
 * - Jun 2022 - Riccardo Maria Bianchi, <riccardo.maria.bianchi@cern.ch>,
 *              Added support for "Verbose" output
 * - Jul 2023 - Riccardo Maria Bianchi, <riccardo.maria.bianchi@cern.ch>,
 *              Populate the caches in the constructor, so all print methods are
 *              available as soon as the input file is opened by the GMDBManager
 */

#include <GeoModelDBManager/GMDBManager.h>

// include the 'fmt' library, which is hosted locally as header-only
#define FMT_HEADER_ONLY 1  // to use 'fmt' header-only
#include "fmt/format.h"

// include SQLite
#include <sqlite3.h>

// C++ includes
#include <stdlib.h> /* exit, EXIT_FAILURE */

#include <mutex>
#include <sstream>

static std::string dbversion =
    "0.6.0";  // Added new tables to store lists of published FullPhysVols and
              // AlignableTransforms

//// FIXME: move this to utility class/file
// std::vector<std::string> toStdVectorStrings(QStringList qlist)
//{
//   std::vector<std::string> vec;
//   foreach(QString qstr, qlist) {
//     vec.push_back(qstr.toStdString());
//   }
//   return vec;
// }

// FIXME: TODO: move to an utility class
std::string getEnvVar(std::string const& key) {
    char* val = std::getenv(key.c_str());
    return val == NULL ? std::string("") : std::string(val);
}

// FIXME: should go to an utility class
std::string joinVectorStrings(std::vector<std::string> vec,
                              std::string sep = "") {
    std::string s;
    unsigned int ii = 0;
    for (const auto& piece : vec) {
        ++ii;
        if (ii == vec.size()) {
            s += (piece);
        } else {
            s += (piece + sep);
        }
    }
    return s;
}

class GMDBManager::Imp {
   public:
    // constructor
    Imp(GMDBManager* dbm)
        : theManager(dbm), m_dbSqlite(nullptr), m_SQLiteErrMsg(0) {}

    // The class
    GMDBManager* theManager;

    // Pointer to SQLite connection
    sqlite3* m_dbSqlite;

    /// Variable to store error messages from SQLite
    char* m_SQLiteErrMsg;

    sqlite3_stmt* selectAllFromTable(std::string tableName) const;
    sqlite3_stmt* selectAllFromTableSortBy(std::string tableName,
                                           std::string sortColumn = "") const;
    sqlite3_stmt* selectAllFromTableChildrenPositions() const;
    bool checkTable_imp(std::string tableName) const;
};

GMDBManager::GMDBManager(const std::string& path)
    : m_dbpath(path), m_dbIsOK(false), m_debug(false), m_d(new Imp(this)) {
    // Check if the user asked for running in serial or multi-threading mode
    if ("" != getEnvVar("GEOMODEL_ENV_IO_DBMANAGER_DEBUG")) {
        m_debug = true;
        std::cout << "You defined the GEOMODEL_IO_DEBUG variable, so you will "
                     "see a verbose output."
                  << std::endl;
    }

    m_dbIsOK = false;

    // FIXME: TODO: we should check the existence of the file, otherwise SQLite
    // will create a new file from scratch

    // Save the connection result
    int exit = sqlite3_open(path.c_str(), &m_d->m_dbSqlite);

    // Test if there was an error
    if (exit == SQLITE_OK) {
        std::cout << "The Geometry Database '" << path
                  << "' has been opened successfully!" << std::endl;
        m_dbIsOK = true;
    } else {
        std::cout << "DB Open Error: " << sqlite3_errmsg(m_d->m_dbSqlite)
                  << std::endl;
        m_dbIsOK = false;
    }

    // set verbosity level
    m_verbose = 0;
    if (const char* env_p = std::getenv("GEOMODEL_GEOMODELIO_VERBOSE")) {
        std::cout << "GeoModelDBManager -- You set the verbosity level to: "
                  << env_p << '\n';
        m_verbose = std::stoi(env_p);
    }

    /// get info from the input DB, if populated,
    /// and create caches storing those pieces of information
    /// This call is only useful when reading an existing DB file.
    /// If the file is new (that is, when we use GMDBManager to write a piece of
    /// geometry into a new file, the call below does nothing.
    createTableDataCaches();
}

GMDBManager::~GMDBManager() {
    sqlite3_close(m_d->m_dbSqlite);
    m_d->m_dbSqlite = nullptr;
    delete m_d;
    m_d = nullptr;
}

void GMDBManager::printAllMaterials() const { printAllRecords("Materials"); }
void GMDBManager::printAllElements() const { printAllRecords("Elements"); }

void GMDBManager::printAllShapes() const { printAllRecords("Shapes"); }

void GMDBManager::printAllSerialDenominators() const {
    printAllRecords("SerialDenominators");
}

void GMDBManager::printAllSerialIdentifiers() const {
    printAllRecords("SerialIdentifiers");
}

void GMDBManager::printAllIdentifierTags() const {
    printAllRecords("IdentifierTags");
}

void GMDBManager::printAllLogVols() const { printAllRecords("LogVols"); }
void GMDBManager::printAllPhysVols() const { printAllRecords("PhysVols"); }
void GMDBManager::printAllFullPhysVols() const {
    printAllRecords("FullPhysVols");
}
void GMDBManager::printAllSerialTransformers() const {
    printAllRecords("SerialTransformers");
}
void GMDBManager::printAllFunctions() const { printAllRecords("Functions"); }
void GMDBManager::printAllTransforms() const { printAllRecords("Transforms"); }
void GMDBManager::printAllAlignableTransforms() const {
    printAllRecords("AlignableTransforms");
}
void GMDBManager::printAllNameTags() const { printAllRecords("NameTags"); }
void GMDBManager::printAllPublishedFullPhysVols(
    const std::string suffix) const {
    if ("" == suffix)
        printAllRecords("PublishedFullPhysVols");
    else {
        std::string tableName = "PublishedFullPhysVols";
        tableName += "_";  // separator
        printAllRecords(tableName + suffix);
    }
}
void GMDBManager::printAllPublishedAlignableTransforms(
    const std::string suffix) const {
    if ("" == suffix)
        printAllRecords("PublishedAlignableTransforms");
    else {
        std::string tableName = "PublishedAlignableTransforms";
        tableName += "_";  // separator
        printAllRecords(tableName + suffix);
    }
}
void GMDBManager::printAllChildrenPositions() const {
    printAllRecords("ChildrenPositions");
}
void GMDBManager::printAllNodeTypes() const {
    printAllRecords("GeoNodesTypes");
}
void GMDBManager::printRootVolumeId() const { printAllRecords("RootVolume"); }
void GMDBManager::printDBVersion() const { printAllRecords("dbversion"); }

void GMDBManager::createTableDataCaches() {
    getAllDBTables();
    getAllDBTableColumns();
}

void GMDBManager::printAllRecords(const std::string& tableName) const {
    // --- print table name
    std::cout << "---" << std::endl;
    std::cout << "'" << tableName << "' in db:" << std::endl;
    // --- check if we stored table's data
    if (m_tableNames.find(tableName) == m_tableNames.end()) {
        std::cout << "\n*** ERROR!! No table '" << tableName
                  << "' found! ***\n\n";
        return;
    }
    // --- print table column names
    std::cout << "- " << joinVectorStrings(m_tableNames.at(tableName), ", ")
              << std::endl;
    // --- print records
    std::vector<std::vector<std::string>> records;
    //  std::vector<std::string> nodeParams;
    records = getTableRecords(tableName);
    if (records.size()) {
        for (auto& row : records) {
            std::cout << "* ";
            for (auto& item : row) {
                std::cout << item << ", ";
            }
            std::cout << std::endl;
        }
    }
    std::cout << "---" << std::endl;
    // TODO: I want to have a symbol like '---' to mean empty line when
    // query gives 0 results.
}

// FIXME: TODO: we now return all records as text, but should we get
// double/int instead when appropriate? In that case, we should create
// dedicated methods for all tables, I guess.
// TODO: fill a cache and returns that if asked a second time
std::vector<std::vector<std::string>> GMDBManager::getTableRecords(
    std::string tableName) const {
    // container to be returned
    std::vector<std::vector<std::string>> records;
    // get the query statetement ready to be executed
    sqlite3_stmt* stmt = nullptr;

    if ("ChildrenPositions" == tableName) {
        stmt = m_d->selectAllFromTableChildrenPositions();
    } else {
        stmt = m_d->selectAllFromTable(tableName);
    }
    // execute the query and loop over all rows and all columuns
    if (stmt) {
        int ctotal = sqlite3_column_count(
            stmt);  // Count the Number of Columns in the Table
        int res = 0;
        while (1) {
            res = sqlite3_step(stmt);  // Execute SQL Statement.
            if (res == SQLITE_ROW) {
                std::vector<std::string>
                    nodeParams;  // stores the data items contained in a
                                 // single row
                for (int i = 0; i < ctotal;
                     i++)  // Loop times the number of columns in the table
                {
                    std::string s;
                    const char* cc = (char*)sqlite3_column_text(
                        stmt,
                        i);  // Read each Column in the row as text FIXME:
                             // is there a method to get the right type,
                             // e.g. double, instead of text?
                    if (cc == NULL)
                        s = "NULL";
                    else
                        s = cc;
                    nodeParams.push_back(s);
                }
                records.push_back(nodeParams);
            }

            if (res == SQLITE_DONE || res == SQLITE_ERROR) {
                if (res == SQLITE_ERROR) {
                    std::string errmsg(sqlite3_errmsg(m_d->m_dbSqlite));
                    sqlite3_finalize(stmt);
                    throw errmsg;
                }
                break;
            }
        }
    }
    // finalize
    sqlite3_finalize(stmt);
    return records;
}

std::vector<std::vector<std::string>> GMDBManager::getTableFromNodeType(
    std::string nodeType) {
    std::vector<std::vector<std::string>> out;
    std::string tableName = getTableNameFromNodeType(nodeType);
    if (tableName.empty()) {
        std::mutex coutMutex;
        coutMutex.lock();
        printf(
            "\t ===> WARNING! The geometry input file does not contain a "
            "table "
            "for the %s nodes. That means that you are probably using an "
            "old "
            "geometry file. Unless you know exactly what you are doing, "
            "please "
            "expect to see incomplete geometries or crashes.\n",
            nodeType.c_str());
        coutMutex.unlock();
    } else {
        out = getTableRecords(tableName);
    }
    return out;
}

// TODO: simplify error reporting for SQLite
// void GMDBManager::showError(const QSqlError &err) const
//{
//  qWarning() << "Unable to initialize Database" << "Error initializing
//  database: " + err.text();
//}

bool GMDBManager::addListOfChildrenPositions(
    const std::vector<std::vector<std::string>>& records) {
    if (records.size() > 0) {
        // NOTE: Choose the right function for your version of SQLite!!
        return addListOfRecordsToTable("ChildrenPositions",
                                       records);  // needs SQLite >= 3.7.11
        // return addListOfRecordsToTableOld("ChildrenPositions", records);
        // // old SQLite versions
    }
    return false;
}

bool GMDBManager::addListOfPublishedAlignableTransforms(
    const std::vector<std::vector<std::string>>& records,
    std::string suffix /* optional parameter */) {
    std::string tableName =
        "PublishedAlignableTransforms";  // default table name
    std::string nodeType = "GeoAlignableTransform";
    const std::type_info& keyType(
        typeid(std::string));  // TODO: type should be custom too!!
    if ("" != suffix) {
        tableName += "_";
        tableName += suffix;
        // debug msg
        // std::cout << "\nSaving the published '"<< nodeType << "' nodes to
        // the custom table: '"
        //          << tableName << "'." << std::endl;
    }
    // create custom table first then add to it
    createTableCustomPublishedNodes(tableName, nodeType, &keyType);
    return addListOfRecordsToTable(tableName,
                                   records);  // needs SQLite >= 3.7.11
    // return addListOfRecordsToTableOld( tableName, records ); // old
    // SQLite versions
}

bool GMDBManager::addListOfPublishedFullPhysVols(
    const std::vector<std::vector<std::string>>& records,
    std::string suffix /* optional parameter */) {
    std::string tableName = "PublishedFullPhysVols";  // default table name
    std::string nodeType = "GeoFullPhysVol";
    const std::type_info& keyType(
        typeid(std::string));  // TODO: type should be custom too!!
    if ("" != suffix) {
        tableName += "_";
        tableName += suffix;
        // std::cout << "\nSaving the published '"<< nodeType << "' nodes to
        // the custom table: '"
        //           << tableName << "'." << std::endl;
    }
    // create custom table first then add to it
    createTableCustomPublishedNodes(tableName, nodeType, &keyType);
    return addListOfRecordsToTable(tableName,
                                   records);  // needs SQLite >= 3.7.11
    // return addListOfRecordsToTableOld( tableName, records ); // old
    // SQLite versions
}

/*
bool GMDBManager::addListOfRecordsToCustomTable(const
std::vector<std::vector<std::string>> &records, std::string tableName )
{
    std::string nodeType = "GeoFullPhysVol";
    const std::type_info &keyType(typeid(std::string));//TODO: type should
be custom too!!
    // create custom table first
    createTableCustomPublishedNodes( tableName, nodeType, &keyType );
    // add records to the newly-created table
    return addListOfRecordsToTable( tableName, records ); // needs SQLite
>= 3.7.11
    //return addListOfRecordsToTableOld( tableName, records ); // old SQLite
versions
}
*/

bool GMDBManager::addListOfRecords(
    const std::string geoType,
    const std::vector<std::vector<std::string>> records) {
    //  if (m_debug) qDebug() << "GMDBManager::addListOfRecords():" <<
    //  geoType;

    std::string tableName = m_childType_tableName[geoType];

    if (tableName.size() == 0) {
        // qWarning() << "m_childType_tableName:" << m_childType_tableName;
        std::cout << "ERROR!! could not retrieve tableName for node type '"
                  << geoType << "'!! Aborting..." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (records.size() > 0) {
        // NOTE: Choose the right function!!
        return addListOfRecordsToTable(
            tableName, records);  // better, but needs SQLite >= 3.7.11
        // return addListOfRecordsToTableOld(tableName, records); // old
        // SQLite versions
    } else {
        if (m_debug)
            std::cout << "Info: no records to save for geoType '" << geoType
                      << "'. Skipping..." << std::endl;
    }
    return true;
}

// ***Note***
// The syntax built here below is more convenient
// but it is supported only in SQLite >= 3.7.11
// (Note: old SLC6-based Athena releases uses SQLite 3.7.9 from LCG)
//
// Here we build a query like this:
// queryStr = QString("INSERT INTO Materials (id, name) VALUES  (1,'Air'),
// (2,'Silicon'), (368,'ShieldSteel');");
//
bool GMDBManager::addListOfRecordsToTable(
    const std::string tableName,
    const std::vector<std::vector<std::string>> records) {
    // get table columns and format them for query
    std::string tableColString =
        "(" + joinVectorStrings(m_tableNames.at(tableName), ", ") + ")";
    if (m_debug) std::cout << "tableColString:" << tableColString << std::endl;

    unsigned int nRecords = records.size();
    std::cout << "Info: number of " << tableName
              << " records to dump into the DB: " << nRecords << std::endl;

    // preparing the SQL query
    std::string sql =
        fmt::format("INSERT INTO {0} {1} VALUES ", tableName, tableColString);
    unsigned int id = 0;
    for (const std::vector<std::string>& rec : records) {
        ++id;
        std::vector<std::string> items;
        for (const std::string& item : rec) {
            items.push_back(
                "'" + item +
                "'");  // TODO: we should differentiate strings from other
                       // values when inserting them in the table, as we now
                       // do for the std::variant version!
        }
        std::string values = joinVectorStrings(items, ",");
        sql += " (" + std::to_string(id) + "," + values + ")";
        if (id != nRecords) {
            sql += ",";
        } else {
            sql += ";";
        }
    }
    if (m_debug) std::cout << "Query string:" << sql << std::endl;  // debug

    // executing the SQL query
    if (!(execQuery(sql))) {
        return false;
    }
    return true;
}

// TODO: use this with std::variant to replace the version with std::string
// only, here above, for all tables; so we can store nativel values (int,
// double, etc...) -- R.M.B.
bool GMDBManager::addListOfRecordsToTable(
    const std::string tableName,
    const std::vector<
        std::vector<std::variant<int, long, float, double, std::string>>>
        records) {
    // get table columns and format them for query
    std::string tableColString =
        "(" + joinVectorStrings(m_tableNames.at(tableName), ", ") + ")";
    if (m_debug) std::cout << "tableColString:" << tableColString << std::endl;

    unsigned int nRecords = records.size();
    std::cout << "Info: number of " << tableName
              << " records to dump into the DB:" << nRecords << std::endl;

    // preparing the SQL query
    std::string sql =
        fmt::format("INSERT INTO {0} {1} VALUES ", tableName, tableColString);
    unsigned int id = 0;
    for (const std::vector<std::variant<int, long, float, double, std::string>>&
             rec : records) {
        ++id;
        // a vector to store string-conversions of values, to build the SQL
        // query
        std::vector<std::string> items;
        // loop over all entries in a row/record
        for (const std::variant<int, long, float, double, std::string>& item :
             rec) {
            if (std::holds_alternative<int>(item))
                items.push_back(std::to_string(std::get<int>(
                    item)));  // we need to encapsulate records' values into
                              // quotes for the SQL query string
            else if (std::holds_alternative<long>(item))
                items.push_back(std::to_string(std::get<long>(item)));
            else if (std::holds_alternative<float>(item))
                items.push_back(std::to_string(std::get<float>(item)));
            else if (std::holds_alternative<double>(item))
                items.push_back(std::to_string(std::get<double>(item)));
            else if (std::holds_alternative<std::string>(item)) {
                std::string str = std::get<std::string>(item);
                // NOTE: if item is a "NULL" string, we don't encapsulate it
                // into quotes, so it is taken as the SQL's NULL value in
                // the SQL string, and inserted as a NULL value in the
                // table, instead of as a "NULL" text string
                if (str == "NULL")
                    items.push_back(str);
                else
                    items.push_back("'" + str + "'");
            } else
                throw std::runtime_error(
                    "No std::variant alternative found!\n");
        }
        // we build the long string containing all values
        std::string values = joinVectorStrings(items, ",");
        sql += " (" + std::to_string(id) + "," + values + ")";
        if (id != nRecords) {
            sql += ",";
        } else {
            sql += ";";
        }
    }
    if (m_debug) std::cout << "Query string:" << sql << std::endl;  // debug

    // executing the SQL query
    if (!(execQuery(sql))) {
        return false;
    }
    return true;
}

// TODO: this is for the old SQLite. Not needed anymore, I guess. ==> Just
// put a requirement on the newer version of SQLite3 in CMakeLists.txt.
// Perhaps, also check that GeoModelIO can run smoothly on older ATLAS
// releases, like 21.9 by taking a newer SQLite3 from LCG.
// ***Note***
// old syntax, for SQLite in SLC6
// here below we build the syntax for multiple INSERT
// compatible with old SQLite versions
// see: stackoverflow.com/questions/1609637
//
// we want to build a query like this:
//  queryStr = QString("INSERT INTO Materials (id, name) SELECT 1 as id,
//  'Air' as name UNION ALL SELECT 2,'Silicon' UNION ALL SELECT
//  368,'ShieldSteel' ")
//
/*
bool GMDBManager::addListOfRecordsToTableOld(const QString tableName, const
std::vector<QStringList> records)
{
        // get table columns and format them for query
  std::string tabColNames =
joinVectorStrings(m_tableNames.at(tableName.toStdString()), ", "); QString
tableColString = "(" + QString::fromStdString(tabColNames) + ")";



    QStringList colNames = m_tableNames.at(tableName);
    unsigned int nRecords = records.size();
    qInfo() << "number of " << tableName << "records to insert into the DB:"
<< nRecords;


     // SQLite has a limit on 'union' items, set at 500. So we have to split
the items if we have more.
     // See: stackoverflow.com/questions/9527851/

    unsigned int bunchSize = 500;
    if ( nRecords > bunchSize ) {
        qWarning() << "WARNING! " << nRecords << " records of type " <<
tableName << "to store in the DB in one call! Call limit is " << bunchSize
<< "
--> We split them in bunches...";

        std::vector<QStringList> recordsCopy( records ); // TODO: maybe we
should use a deque or queue, which have fast pop of first element?

        std::vector<QStringList>::const_iterator first;
        std::vector<QStringList>::const_iterator last;

        unsigned int start = 0;

        while ( recordsCopy.size() > 0 ) {

        // preparing the SQL query
        QString queryStr("INSERT INTO %1 %2 SELECT ");
            queryStr = queryStr.arg(tableName); // insert table name
            queryStr = queryStr.arg(tableColString); // insert table columns

        first = recordsCopy.begin();
        last  = recordsCopy.size() > bunchSize ? recordsCopy.begin() +
bunchSize : recordsCopy.end();

        std::vector<QStringList> recordsBunch( first, last ); // we take the
first 500 records recordsCopy.erase( first, last ); // we delete the first
500 records if (m_debug) qDebug() << "start:" << start << "recordsBunch
size:" << recordsBunch.size() << "- recordsCopy size after removal:" <<
recordsCopy.size();

            // --- first record
        // outcome should be like: " 1 as id, 'Air' as name "
        unsigned int id = start+1; // set the first ID as 'start'
        QStringList recFirst = recordsBunch.front(); // access first record
from vector recordsBunch.erase( recordsBunch.begin() ); // delete first
record if (m_debug) qDebug() << "after taking first record - recordsBunch
size:" << recordsBunch.size();

        // first item in the first record (it is the ID)
        QString firstCol = colNames[0];
        queryStr += QString::number(id) + " as " + firstCol + ", ";

        // the other items in the first record
        unsigned int ii = 1;
        unsigned int nRecs = recFirst.size();
        foreach (QString rec, recFirst) {
            queryStr += "'" + rec + "'" + " as " + colNames[ii];
            if (ii != nRecs)
                queryStr += ","; // add coma, but not on latest
            queryStr += " "; // add space
            //qDebug() << "first element:" << ii << nRecs << queryStr;
            ++ii;
        }
        if (m_debug) qDebug() << "first element query:" << queryStr;
        // --- other records
        // outcome should be: " UNION ALL SELECT 2,'Silicon' "


            foreach(QStringList recs, recordsBunch) {

                                                        // DEBUG
                                                        if (tableName ==
"Functions") { if (recs[1].length() > 65000) { std::cout << "LONG STRING!
size: " << recs[1].length() << std::endl; std::cout << "LONG STRING! string:
" << recs[1].toStdString() << std::endl << std::endl;
                                                                }
                                                        }
                                                        //------

                                                                ++id;
                // put single quotes around items
                QStringList items;
                foreach (QString item, recs) {
                                                items << "'" + item + "'";
                                        }
                // join items into a string and add the SQL commands
                QString itemsStr = items.join(", ");
                queryStr += " UNION ALL SELECT " + QString::number(id) + ",
" + itemsStr;
            }

        //qDebug() << "queryStr:" << queryStr;

            // executing the SQL query
            QSqlQuery q;
            if (!q.exec(queryStr)) {
            qWarning() << "ERROR!!! SQL error:";
                    showError(q.lastError());
                    return false;
            }
        // JFB commented: qDebug() << bunchSize << "elements have been saved
into the DB, starting at:" << start;

        start += bunchSize; // for the next iteration

      } // end of while

    } // end of if(>500)
    else {
    // preparing the SQL query
    QString queryStr("INSERT INTO %1 %2 SELECT ");
        queryStr = queryStr.arg(tableName); // insert table name
        queryStr = queryStr.arg(tableColString); // insert table columns

    unsigned int id = 0;
    // loop on all records
        foreach(QStringList recs, records) {

                ++id;

        // first record
        // outcome should be like: " 1 as id, 'Air' as name "
        if (id == 1) {

            // first item if the record (it is the ID)
            QString firstCol = colNames.takeFirst();
            queryStr += QString::number(id) + " as " + firstCol + ", ";

            // the other items in the record
            unsigned int ii = 0;
            unsigned int nRecs = recs.size();
            foreach (QString rec, recs) {
                queryStr += "'" + rec + "'" + " as " + colNames[ii];
                if (ii != nRecs-1)
                    queryStr += ","; // add coma, but not on latest
                queryStr += " "; // add space
                //qDebug() << ii << nRecs << queryStr;
                ++ii;
            }

         }
        // other records
        // outcome should be: " UNION ALL SELECT 2,'Silicon' "
        else {

            // put single quotes around items
            QStringList items;
            foreach (QString item, recs) {
                            items << "'" + item + "'";
                    }
            // join items into a string and add the SQL commands
            QString itemsStr = items.join(", ");
            queryStr += " UNION ALL SELECT " + QString::number(id) + ", " +
itemsStr;
        }

        } // end of: foreach(QStringList recs, records)

    //qDebug() << "queryStr:" << queryStr;

        // executing the SQL query
        QSqlQuery q;
        if (!q.exec(queryStr)) {
        qWarning() << "ERROR!!! SQL error:";
                showError(q.lastError());
                return false;
        }
} // end of (else > 500)

        // JFB commented: qDebug() << "DONE. The list of " << nRecords <<
"records have been inserted into the DB."; return true;

}
*/

bool GMDBManager::addRootVolume(const std::vector<std::string>& values) {
    if (values.size() > 0) {
        const unsigned int volId = std::stoi(values[0]);
        const std::string nodeType = values[1];
        return storeRootVolume(volId, nodeType);
    } else {
        std::cout << "ERROR! No records to save for RootVolume! All GeoModel "
                     "trees must have a 'World'/'Root' volume! Exiting..."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
}

void GMDBManager::addDBversion(std::string version) {
    checkIsDBOpen();
    sqlite3_stmt* st = nullptr;
    int rc = -1;
    std::string sql = "INSERT INTO dbversion(version) VALUES(?)";
    rc = sqlite3_prepare_v2(m_d->m_dbSqlite, sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) {
        printf("[SQLite ERR] (%s) : Error msg: %s\n", __func__,
               sqlite3_errmsg(m_d->m_dbSqlite));
        exit(EXIT_FAILURE);
    }
    rc = sqlite3_bind_text(st, 1, version.c_str(), version.length(),
                           SQLITE_TRANSIENT);
    rc = sqlite3_step(st);
    if (rc != SQLITE_DONE) {
        printf("[SQLite ERR] (%s) : Error msg: %s\n", __func__,
               sqlite3_errmsg(m_d->m_dbSqlite));
        exit(EXIT_FAILURE);
    }
    // finalize
    sqlite3_finalize(st);
    return;
}

bool GMDBManager::checkIsDBOpen() const {
    if (m_d->m_dbSqlite != nullptr) {
        return true;
    } else {
        std::cout << "ERROR! The SQLite DB is not accessible! Exiting..."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::vector<std::string> GMDBManager::getItemAndType(unsigned int tableId,
                                                     unsigned int id) {
    std::vector<std::string> results;
    std::string tableName = getTableNameFromTableId(tableId);
    std::string nodeType = getNodeTypeFromTableId(tableId);
    std::vector<std::string> item = getItemFromTableName(tableName, id);
    // add the type
    results.push_back(nodeType);
    // add the item
    results.insert(results.end(), item.begin(), item.end());
    return results;
}

std::vector<std::string> GMDBManager::getItemFromTableName(
    std::string tableName, unsigned int id) {
    // FIXME: when you create caches for all tables, replace this method
    // with a lookup action in the cache.
    /*
     * Get the object from DB.
     */
    std::vector<std::string> item;
    // set a SQL command string with the right table name
    std::string sql =
        fmt::format("SELECT * FROM {0} WHERE id = (?)", tableName);
    // prepare the query
    sqlite3_stmt* stmt = nullptr;
    int rc = -1;
    rc = sqlite3_prepare_v2(m_d->m_dbSqlite, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[SQLite ERR] 'prepare' (%s) : Error msg: %s\n", __func__,
               sqlite3_errmsg(m_d->m_dbSqlite));
        exit(EXIT_FAILURE);
    }
    // bind the parameters
    rc = sqlite3_bind_int(stmt, 1, id);
    // execute the query and loop over all rows and all columuns
    if (stmt) {
        int ctotal = sqlite3_column_count(
            stmt);  // Count the Number of Columns in the Table
        int res = 0;
        while (1) {
            res = sqlite3_step(stmt);  // Execute SQL Statement.
            if (res == SQLITE_ROW) {
                for (int i = 0; i < ctotal;
                     i++)  // Loop times the number of columns in the table
                {
                    std::string s = (char*)sqlite3_column_text(
                        stmt,
                        i);  // Read each Column in the row as text FIXME:
                             // is there a method to get the right type,
                             // e.g. double, instead of text?
                    item.push_back(s);
                }
            }
            if (res == SQLITE_DONE || res == SQLITE_ERROR) {
                if (res == SQLITE_ERROR) {
                    std::string errmsg(sqlite3_errmsg(m_d->m_dbSqlite));
                    sqlite3_finalize(stmt);
                    throw errmsg;
                }
                break;
            }
        }
    }
    // TODO: do we need that error check here??
    //  if (rc != SQLITE_DONE) {
    //    printf( "[SQLite ERR] 'step' (%s) : Error msg: %s\n", __func__,
    //    sqlite3_errmsg(m_d->m_dbSqlite) );
    ////    exit(EXIT_FAILURE);
    //  }
    // finalize
    sqlite3_finalize(stmt);

    if (item.size() == 0) {
        std::cout << "ERROR!!"
                  << "Item with ID:'" << id << "' does not exist in table"
                  << tableName << "! Exiting...";
        exit(EXIT_FAILURE);
    }

    return item;
}

// Get all parent-children data from the database in one go
std::vector<std::vector<std::string>> GMDBManager::getChildrenTable() {
    return getTableRecords("ChildrenPositions");
}

unsigned int GMDBManager::getTableIdFromNodeType(const std::string& nodeType) {
    return m_cache_nodeType_tableID[nodeType];
}

std::string GMDBManager::getTableNameFromNodeType(const std::string& nodeType) {
    std::string st{""};
    if (m_cache_nodeType_tableName.count(nodeType)) {
        st = m_cache_nodeType_tableName.at(nodeType);
    } else {
        std::mutex coutMutex;
        coutMutex.lock();
        std::cout << "\t ===> WARNING! A table for nodeType '" << nodeType
                  << "' has not been found in the input geometry file."
                  << std::endl;
        coutMutex.unlock();
    }
    return st;
}

// build the GeoNodeTypes cache
int GMDBManager::loadGeoNodeTypesAndBuildCache() {
    checkIsDBOpen();
    std::string queryStr;
    sqlite3_stmt* st = nullptr;
    int rc = -1;
    // set the SQL query string
    std::string sql = "SELECT * FROM GeoNodesTypes";
    // declare the data we want to fetch
    unsigned int id = 0;
    std::string nodeType = "";
    std::string tableName = "";
    // prepare the query
    rc = sqlite3_prepare_v2(m_d->m_dbSqlite, sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) {
        printf("[SQLite ERR] (%s) : Error msg: %s\n", __func__,
               sqlite3_errmsg(m_d->m_dbSqlite));
        exit(EXIT_FAILURE);
    }
    // execute the statement until all selected records are processed
    while ((rc = sqlite3_step(st)) == SQLITE_ROW) {
        // get the data
        id = sqlite3_column_int(st, 0);
        nodeType = std::string(
            reinterpret_cast<const char*>(sqlite3_column_text(st, 1)));
        tableName = std::string(
            reinterpret_cast<const char*>(sqlite3_column_text(st, 2)));
        if (m_debug)
            std::cout << "row: " << id << "," << nodeType << "," << tableName
                      << std::endl;
        // fill the caches
        m_cache_tableId_tableName.insert(
            std::pair<unsigned int, std::string>(id, tableName));
        m_cache_tableId_nodeType.insert(
            std::pair<unsigned int, std::string>(id, nodeType));
        m_cache_nodeType_tableName.insert(
            std::pair<std::string, std::string>(nodeType, tableName));
        m_cache_nodeType_tableID.insert(
            std::pair<std::string, unsigned int>(nodeType, id));
    }
    if (rc != SQLITE_DONE) {
        std::string errmsg(sqlite3_errmsg(m_d->m_dbSqlite));
        sqlite3_finalize(st);
        throw errmsg;
    }
    // finalize
    sqlite3_finalize(st);
    return rc;
}

// FIXME: should throw an error if the method
// `loadGeoNodeTypesAndBuildCache()` was not called before calling this
// method
std::string GMDBManager::getTableNameFromTableId(unsigned int tabId) {
    return m_cache_tableId_tableName[tabId];
}

std::string GMDBManager::getNodeTypeFromTableId(unsigned int tabId) {
    return m_cache_tableId_nodeType[tabId];
}

std::unordered_map<unsigned int, std::string>
GMDBManager::getAll_TableIDsNodeTypes() {
    return m_cache_tableId_nodeType;
}

std::unordered_map<std::string, unsigned int>
GMDBManager::getAll_NodeTypesTableIDs() {
    return m_cache_nodeType_tableID;
}

sqlite3_stmt* GMDBManager::Imp::selectAllFromTable(
    std::string tableName) const {
    return selectAllFromTableSortBy(tableName, "id");
}

sqlite3_stmt* GMDBManager::Imp::selectAllFromTableSortBy(
    std::string tableName, std::string sortColumn) const {
    theManager->checkIsDBOpen();
    if ("" == sortColumn || 0 == sortColumn.size()) {
        sortColumn = "id";
    }
    sqlite3_stmt* st = nullptr;  // SQLite statement to be returned
    int rc = -1;                 // SQLite return code
    // set the SQL query string
    std::string sql =
        fmt::format("SELECT * FROM {0} ORDER BY {1}", tableName, sortColumn);
    // prepare the query
    rc = sqlite3_prepare_v2(m_dbSqlite, sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) {
        printf("[SQLite ERR] (%s) : Error msg: %s\n", __func__,
               sqlite3_errmsg(m_dbSqlite));
        exit(EXIT_FAILURE);
    }
    return st;
}

bool GMDBManager::Imp::checkTable_imp(std::string tableName) const {
    theManager->checkIsDBOpen();
    sqlite3_stmt* st = nullptr;  // SQLite statement to be returned
    int rc = -1;                 // SQLite return code
    // set the SQL query string
    std::string sql = fmt::format("SELECT * FROM {0}", tableName);
    // prepare the query
    rc = sqlite3_prepare_v2(m_dbSqlite, sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) return false;
    return true;
}

sqlite3_stmt* GMDBManager::Imp::selectAllFromTableChildrenPositions() const {
    theManager->checkIsDBOpen();
    sqlite3_stmt* st = nullptr;
    int rc = -1;
    // set the SQL query string
    std::string sql =
        "SELECT * FROM ChildrenPositions ORDER BY parentTable, parentId, "
        "parentCopyNumber, position";
    // prepare the query
    rc = sqlite3_prepare_v2(m_dbSqlite, sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) {
        printf("[SQLite ERR] (%s) : Error msg: %s\n", __func__,
               sqlite3_errmsg(m_dbSqlite));
        exit(EXIT_FAILURE);
    }
    return st;
}

bool GMDBManager::initDB() {
    // create tables
    bool tablesOK = createTables();
    // store DB version
    addDBversion(dbversion);
    return tablesOK;
}

void GMDBManager::printAllDBTables() {
    if (m_cache_tables.size() == 0) {
        getAllDBTables();  // load tables and build the cache
    }
    for (auto& str : m_cache_tables) std::cout << str << std::endl;
}

void GMDBManager::getAllDBTables() {
    std::string tableName;
    std::vector<std::string> tables;
    // define a query string containing the necessary SQL instructions
    std::string queryStr =
        "SELECT name FROM sqlite_master WHERE type ='table' AND name NOT "
        "LIKE "
        "'sqlite_%';";
    // prepare the query with the query string
    sqlite3_stmt* stmt;
    int rc =
        sqlite3_prepare_v2(m_d->m_dbSqlite, queryStr.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        throw std::string(sqlite3_errmsg(m_d->m_dbSqlite));
    }
    // execute the statement until all selected records are processed
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        tableName = std::string(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        // std::cout << "tableName: " << tableName << std::endl; // debug
        tables.push_back(tableName);
    }
    if (rc != SQLITE_DONE) {
        std::string errmsg(sqlite3_errmsg(m_d->m_dbSqlite));
        sqlite3_finalize(stmt);
        throw errmsg;
    }
    // finalize
    sqlite3_finalize(stmt);

    m_cache_tables = tables;
}

void GMDBManager::getAllDBTableColumns() {
    std::vector<std::string> cols;
    std::string colName;

    // populate the cache with tables' names, if needed
    if (!m_cache_tables.size()) {
        std::cout << "*** ERROR! Tables' cache is empty! ***\n";
        return;
    }

    for (auto& tableName : m_cache_tables) {
        sqlite3_stmt* stmt;
        // get the 'name' column from the PRAGMA's table's definition
        // see: https://stackoverflow.com/a/54962853/320369
        std::string queryStr =
            "select name from pragma_table_info('" + tableName + "')";
        int rc = sqlite3_prepare_v2(m_d->m_dbSqlite, queryStr.c_str(), -1,
                                    &stmt, NULL);
        if (rc == SQLITE_OK) {
            // OK, now looping over table's columns...
            // will continue to go down the rows (columns in your table)
            // till there are no more
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                // sprintf(colName, "%s", sqlite3_column_text(stmt, 1));
                colName = std::string(reinterpret_cast<const char*>(
                    sqlite3_column_text(stmt, 0)));

                // do something with colName because it contains the
                // column's name
                m_tableNames[tableName].push_back(colName);
            }
        }
    }  // end of for loop
}

// TODO: currently, we retrieve published data as strings, but we want to
// retrieve that according to the original data type
std::vector<std::vector<std::string>> GMDBManager::getPublishedFPVTable(
    std::string suffix) {
    std::string tableName = "PublishedFullPhysVols";  // default table name
    if ("" != suffix) {
        tableName += "_";
        tableName += suffix;
    }

    return getTableRecords(tableName);
}
// TODO: currently, we retrieve published data as strings, but we want to
// retrieve that according to the original data type
std::vector<std::vector<std::string>> GMDBManager::getPublishedAXFTable(
    std::string suffix) {
    std::string tableName =
        "PublishedAlignableTransforms";  // default table name
    if ("" != suffix) {
        tableName += "_";
        tableName += suffix;
    }

    return getTableRecords(tableName);
}

bool GMDBManager::checkTable(std::string tableName) const {
    return m_d->checkTable_imp(tableName);
}

// create a user-defined custom table to store the published nodes
// (usually GeoFullPhysVol and AlignableTransform nodes) and their keys.
bool GMDBManager::createTableCustomPublishedNodes(
    const std::string tableName, const std::string nodeType,
    const std::type_info* keyType) {
    // get the right node type and referenced table
    if (nodeType != "GeoFullPhysVol" && nodeType != "GeoVFullPhysVol" &&
        nodeType != "GeoAlignableTransform") {
        std::cout << "ERROR!! GeoModel node type '" << nodeType
                  << "' is not currently supported in "
                     "GMDBManager::createTableCustomPublishedNodes()"
                  << " Please, ask to geomodel-developers@cern.ch. Exiting..."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string referencedTable = "";
    if ("GeoFullPhysVol" == nodeType || "GeoVFullPhysVol" == nodeType)
        referencedTable = "FullPhysVols";
    if ("GeoAlignableTransform" == nodeType)
        referencedTable = "AlignableTransforms";

    // set the right type to use in the DB, based on the type used for the
    // published key
    std::string keyTypeDB = "";
    if (typeid(std::string) == *keyType)
        keyTypeDB = "varchar";
    else if (typeid(int) == *keyType || typeid(unsigned) == *keyType)
        keyTypeDB = "integer";
    else {
        std::cout << "ERROR!!! The key type '" << typeid(keyType).name()
                  << "' is not currently supported in "
                     "GMDBManager::createTableCustomPublishedNodes()."
                  << " Please, ask to 'geomodel-developers@cern.ch'. Exiting..."
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    int rc = -1;  // sqlite's return code
    std::string queryStr;

    std::vector<std::string> tab;

    tab.insert(tab.begin(), {tableName, "id", "key", "nodeID", "keyType"});
    storeTableColumnNames(tab);

    queryStr = fmt::format(
        "create table {0} ({1} integer primary key, {2} {3} not null, {4} "
        "integer not null REFERENCES {5}(id), {6} varchar not null)",
        tab[0], tab[1], tab[2], keyTypeDB, tab[3], referencedTable, tab[4]);
    // std::cout << "Creating table with query: '" << queryStr << "'..." <<
    // std::endl; // debug msg
    rc = execQuery(queryStr);
    tab.clear();
    // std::cout << "Created the custom table: '" << tableName << "'." <<
    // std::endl; // debug msg
    return rc;
}

/* --- not used anymore, we now have std::variant, below ---
// create a user-defined custom table to store auxiliary data, from
vector<vector<string>> bool GMDBManager::createCustomTable(const std::string
tableName, const std::vector<std::string> tableColNames, const
std::vector<std::string> tableColTypes, const
std::vector<std::vector<std::string>> &records )
{
    if( tableColNames.size() == 0 ) throw
std::runtime_error("GMDBManager::createCustomTable -- The list of columns'
names is empty!!"); if( tableColTypes.size() == 0 ) throw
std::runtime_error("GMDBManager::createCustomTable -- The list of columns'
types is empty!!");

  int rc = -1; // sqlite's return code
  std::string queryStr;

  std::vector<std::string> tab;

  tab.push_back(tableName);
  tab.push_back( "id" ); // this is the column to store the records' IDs
  for( auto& colName : tableColNames )
    tab.push_back( colName );

  storeTableColumnNames(tab);

  // prepare the dynamic query to create the custom table
  queryStr = fmt::format( "create table {0} ( id integer primary key ",
tab[0]
); for( int ii=0; ii<tableColNames.size(); ++ii) { std::string colStr =
fmt::format( ", {0} {1} ", tableColNames[ii], tableColTypes[ii] ); queryStr
+= colStr;
  }
  queryStr += ")";

  rc = execQuery(queryStr);
  tab.clear();
  return addListOfRecordsToTable( tableName, records ); // needs SQLite
>= 3.7.11
}
*/

// create a user-defined custom table to store auxiliary data, from
// vector<vector<variant>>
bool GMDBManager::createCustomTable(
    const std::string tableName, const std::vector<std::string> tableColNames,
    const std::vector<std::string> tableColTypes,
    const std::vector<
        std::vector<std::variant<int, long, float, double, std::string>>>&
        records) {
    if (tableColNames.size() == 0)
        throw std::runtime_error(
            "GMDBManager::createCustomTable -- The list of columns' names "
            "is "
            "empty!!");
    if (tableColTypes.size() == 0)
        throw std::runtime_error(
            "GMDBManager::createCustomTable -- The list of columns' types "
            "is "
            "empty!!");

    std::string queryStr;

    std::vector<std::string> tab;

    tab.push_back(tableName);
    tab.push_back("id");  // this is the column to store the records' IDs
    for (auto& colName : tableColNames) tab.push_back(colName);

    storeTableColumnNames(tab);

    // prepare the dynamic query to create the custom table
    queryStr =
        fmt::format("create table {0} ( id integer primary key ", tab[0]);
    for (size_t ii = 0; ii < tableColNames.size(); ++ii) {
        std::string colType = "";

        // -- Here we check the datum's type, which is more universal than
        // using string-encoded types
        // -- but this does not work if the first entry of a given column is
        // empty.
        // -- If so, the 'null' entry is taken as string, and the column is
        // set as containing TEXT... convert std::variant types to SQLite
        // data types (see SQLite documentation)
        /*
        if(      std::holds_alternative<int>(records[0][ii]) ) colType =
        "INTEGER"; else if( std::holds_alternative<long>(records[0][ii]) )
        colType = "INTEGER"; else if(
        std::holds_alternative<float>(records[0][ii]) )       colType =
        "REAL"; else if( std::holds_alternative<double>(records[0][ii]) )
        colType = "REAL"; else if(
        std::holds_alternative<std::string>(records[0][ii]) ) colType =
        "TEXT"; else throw std::runtime_error("No std::variant alternative
        has been found!\n");
        */

        if (tableColTypes[ii] == "INT")
            colType = "INTEGER";
        else if (tableColTypes[ii] == "LONG")
            colType = "INTEGER";
        else if (tableColTypes[ii] == "FLOAT")
            colType = "REAL";
        else if (tableColTypes[ii] == "DOUBLE")
            colType = "REAL";
        else if (tableColTypes[ii] == "STRING")
            colType = "TEXT";
        else
            throw std::runtime_error(
                "No suitable column type has been found ==> " +
                tableColTypes[ii] + "\n");

        std::string colStr =
            fmt::format(", {0} {1} ", tableColNames[ii], colType);
        queryStr += colStr;
    }
    queryStr += ")";
    if (m_verbose > 0)
        std::cout << "- table definition: " << queryStr << std::endl;

    (void)execQuery(queryStr);
    tab.clear();
    return addListOfRecordsToTable(tableName,
                                   records);  // needs SQLite >= 3.7.11
}

bool GMDBManager::createTables() {
    checkIsDBOpen();

    int rc = -1;  // sqlite return code
    std::string queryStr;

    std::string geoNode;
    std::string tableName;
    std::vector<std::string> tab;

    // create a table to store the relation between the types of GeoNodes
    // and the name of the table
    tableName = "dbversion";
    tab.insert(tab.begin(), {tableName, "id", "version"});
    storeTableColumnNames(tab);
    queryStr =
        fmt::format("create table {0} ({1} integer primary key, {2} integer)",
                    tab[0], tab[1], tab[2]);
    rc = execQuery(queryStr);
    tab.clear();

    // create a table to store the relation between the types of GeoNodes
    // and the name of the table
    tableName = "GeoNodesTypes";
    tab.insert(tab.begin(), {tableName, "id", "nodeType", "tableName"});
    storeTableColumnNames(tab);
    queryStr = fmt::format(
        "create table {0}({1} integer primary key, {2} varchar, {3} "
        "varchar)",
        tab[0], tab[1], tab[2], tab[3]);
    rc = execQuery(queryStr);
    tab.clear();

    // create a table to store the mother-daughter relationships between
    // nodes (notably, between the [Full]PhysVols as the parents and their
    // children)
    tableName = "ChildrenPositions";
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("parentId");
    tab.push_back("parentTable");
    tab.push_back("parentCopyNumber");
    tab.push_back("position");
    tab.push_back("childTable");
    tab.push_back("childId");
    tab.push_back("childCopyNumber");
    storeTableColumnNames(tab);
    queryStr = fmt::format(
        "create table {0}({1} integer primary key, {2} integer, {3} "
        "integer "
        "not null REFERENCES GeoNodesTypes(id), {4} integer, {5} integer, "
        "{6} "
        "integer not null REFERENCES GeoNodesTypes(id), {7} integer not "
        "null, "
        "{8} integer not null)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7], tab[8]);
    rc = execQuery(queryStr);
    tab.clear();

    // create a table to store information about the 'root' volume (also
    // known as the 'world' volume)
    tableName = "RootVolume";
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("volId");
    tab.push_back("volTable");
    storeTableColumnNames(tab);
    queryStr = fmt::format(
        "create table {0}({1} integer primary key, {2} integer not null, "
        "{3} "
        "integer not null REFERENCES GeoNodesTypes(id))",
        tab[0], tab[1], tab[2], tab[3]);
    rc = execQuery(queryStr);
    tab.clear();

    // PhysVols table
    geoNode = "GeoPhysVol";
    tableName = "PhysVols";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("logvol");
    storeTableColumnNames(tab);
    queryStr = fmt::format(
        "create table {0}({1} integer primary key, {2} integer not null)",
        tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // FullPhysVols table
    geoNode = "GeoFullPhysVol";
    tableName = "FullPhysVols";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("logvol");
    storeTableColumnNames(tab);
    queryStr = fmt::format(
        "create table {0}({1} integer primary key, {2} integer not null)",
        tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // LogVols table
    geoNode = "GeoLogVol";
    tableName = "LogVols";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("name");
    tab.push_back("shape");
    tab.push_back("material");
    storeTableColumnNames(tab);
    queryStr = fmt::format(
        "create table {0}({1} integer primary key, {2} varchar, {3} "
        "integer "
        "not null, {4} integer not null)",
        tab[0], tab[1], tab[2], tab[3], tab[4]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Materials table
    geoNode = "GeoMaterial";
    tableName = "Materials";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("name");
    tab.push_back("density");
    tab.push_back("elements");
    storeTableColumnNames(tab);
    queryStr = fmt::format(
        "create table {0}({1} integer primary key, {2} varchar, {3} "
        "varchar, "
        "{4} varchar)",
        tab[0], tab[1], tab[2], tab[3], tab[4]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Elements table
    geoNode = "GeoElement";
    tableName = "Elements";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("name");
    tab.push_back("symbol");
    tab.push_back("Z");
    tab.push_back("A");
    storeTableColumnNames(tab);
    queryStr = fmt::format(
        "create table {0}({1} integer primary key, {2} varchar, {3} "
        "varchar, "
        "{4} varchar, {5} varchar)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Shapes table
    geoNode = "GeoShape";
    tableName = "Shapes";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("type");
    tab.push_back("parameters");
    storeTableColumnNames(tab);
    queryStr = fmt::format(
        "create table {0}({1} integer primary key, {2} varchar, {3} "
        "varchar)",
        tab[0], tab[1], tab[2], tab[3]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // SerialDenominators table
    geoNode = "GeoSerialDenominator";
    tableName = "SerialDenominators";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("baseName");
    storeTableColumnNames(tab);
    queryStr =
        fmt::format("create table {0}({1} integer primary key, {2} varchar)",
                    tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // SerialIdentifiers table
    geoNode = "GeoSerialIdentifier";
    tableName = "SerialIdentifiers";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("baseId");
    storeTableColumnNames(tab);
    queryStr =
        fmt::format("create table {0}({1} integer primary key, {2} integer)",
                    tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // IdentifierTags table
    geoNode = "GeoIdentifierTag";
    tableName = "IdentifierTags";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("identifier");
    storeTableColumnNames(tab);
    queryStr =
        fmt::format("create table {0}({1} integer primary key, {2} integer)",
                    tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Functions table
    geoNode = "Function";
    tableName = "Functions";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("expression");
    storeTableColumnNames(tab);
    queryStr =
        fmt::format("create table {0}({1} integer primary key, {2} varchar)",
                    tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // SerialTransformers table
    geoNode = "GeoSerialTransformer";
    tableName = "SerialTransformers";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("funcId");
    tab.push_back("volId");
    tab.push_back("volTable");
    tab.push_back("copies");
    storeTableColumnNames(tab);
    queryStr = fmt::format(
        "create table {0}({1} integer primary key, {2} integer not null "
        "REFERENCES Functions(id), {3} integer not null, {4} integer not "
        "null "
        "REFERENCES GeoNodesTypes(id), {5} integer)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Transforms table
    geoNode = "GeoTransform";
    tableName = "Transforms";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("xx");
    tab.push_back("xy");
    tab.push_back("xz");
    tab.push_back("yx");
    tab.push_back("yy");
    tab.push_back("yz");
    tab.push_back("zx");
    tab.push_back("zy");
    tab.push_back("zz");
    tab.push_back("dx");
    tab.push_back("dy");
    tab.push_back("dz");
    storeTableColumnNames(tab);
    queryStr = fmt::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} "
        "real, {5} real, {6} real, {7} real, {8} real, {9} real, {10} "
        "real, "
        "{11} real, {12} real, {13} real)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7], tab[8],
        tab[9], tab[10], tab[11], tab[12], tab[13]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Transforms table
    geoNode = "GeoAlignableTransform";
    tableName = "AlignableTransforms";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("xx");
    tab.push_back("xy");
    tab.push_back("xz");
    tab.push_back("yx");
    tab.push_back("yy");
    tab.push_back("yz");
    tab.push_back("zx");
    tab.push_back("zy");
    tab.push_back("zz");
    tab.push_back("dx");
    tab.push_back("dy");
    tab.push_back("dz");
    storeTableColumnNames(tab);
    queryStr = fmt::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} "
        "real, {5} real, {6} real, {7} real, {8} real, {9} real, {10} "
        "real, "
        "{11} real, {12} real, {13} real)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7], tab[8],
        tab[9], tab[10], tab[11], tab[12], tab[13]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // NameTags table
    geoNode = "GeoNameTag";
    tableName = "NameTags";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("name");
    storeTableColumnNames(tab);
    queryStr =
        fmt::format("create table {0}({1} integer primary key, {2} varchar)",
                    tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    if (m_debug) {
        std::cout << "All these tables have been successfully created:"
                  << std::endl;  // debug
        printAllDBTables();
    }

    if (rc == 0) {
        return true;
    }
    return false;
}

int GMDBManager::execQuery(std::string queryStr) {
    if (m_debug)
        std::cout << "queryStr to execute: " << queryStr << std::endl;  // debug
    checkIsDBOpen();
    int result = -1;
    if ((result = sqlite3_exec(m_d->m_dbSqlite, queryStr.c_str(), NULL, 0,
                               &m_d->m_SQLiteErrMsg))) {
        printf("[ERR] : \t> CMD: %s , Error: %d\n", queryStr.c_str(), result);
        if (m_d->m_SQLiteErrMsg) {
            printf("[ERR] : Error msg: %s\n", m_d->m_SQLiteErrMsg);
            sqlite3_free(m_d->m_SQLiteErrMsg);
        }
    }
    return result;
}

void GMDBManager::storeTableColumnNames(std::vector<std::string> input) {
    if (!(input.size() == 0)) {
        std::string tabName =
            input.at(0);  // get the first element storing the name of the table
        input.erase(input.begin());  // remove the first element storing the
                                     // name of the table
        m_tableNames[tabName] = input;
    }
}

std::vector<std::string> GMDBManager::getTableColumnNames(
    const std::string& tableName) {
    return m_tableNames.at(tableName);
}

void GMDBManager::storeNodeType(std::string nodeType, std::string tableName) {
    checkIsDBOpen();
    std::string queryStr;
    sqlite3_stmt* st = nullptr;
    int rc = -1;
    // preparing the SQL query
    std::string sql =
        "INSERT INTO GeoNodesTypes(nodeType, tableName) VALUES(?, ?)";
    rc = sqlite3_prepare_v2(m_d->m_dbSqlite, sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) {
        printf("[SQLite ERR] (%s) : Error msg: %s\n", __func__,
               sqlite3_errmsg(m_d->m_dbSqlite));
        exit(EXIT_FAILURE);
    }
    if (m_debug)
        std::cout << "storeNodeType - Query string:" << sql
                  << std::endl;  // debug
    // bind the parameters
    rc = sqlite3_bind_text(st, 1, nodeType.c_str(), nodeType.length(),
                           SQLITE_TRANSIENT);
    rc = sqlite3_bind_text(st, 2, tableName.c_str(), tableName.length(),
                           SQLITE_TRANSIENT);
    // execute the query
    rc = sqlite3_step(st);
    if (rc != SQLITE_DONE) {
        printf("[SQLite ERR] (%s) : Error msg: %s\n", __func__,
               sqlite3_errmsg(m_d->m_dbSqlite));
        exit(EXIT_FAILURE);
    }
    // finalize
    sqlite3_finalize(st);
    return;
}

// TODO: move to sqlite3 library
// void GMDBManager::loadTestData()
//{
//  // create test data
//  QSqlQuery q;
//  //
//  //  qDebug() << "Loading Shapes...";
//  //  if (!q.prepare(QLatin1String("insert into Shapes(name) values(?)")))
//  {
//  //    showError(q.lastError());
//  //    return;
//  //  }
//  QVariant boxId    = addShape(QLatin1String("Box"), "");
//  QVariant coneId   = addShape(QLatin1String("Cone"), "");
//  QVariant sphereId = addShape(QLatin1String("Sphere"), "");
//
//  qWarning() << "Sample GeoElement data --> to be implemented!";
//
//  QVariant airId  =
//  addMaterial(QLatin1String("Air"),QLatin1String("density"),QLatin1String("elements"));
//  QVariant ironId =
//  addMaterial(QLatin1String("Iron"),QLatin1String("density"),QLatin1String("elements"));
//  QVariant leadId =
//  addMaterial(QLatin1String("Lead"),QLatin1String("density"),QLatin1String("elements"));
//
//  QVariant worldLogId = addLogVol(QLatin1String("WorldLog"), boxId,
//  airId); QVariant toyLogId  = addLogVol(QLatin1String("ToyLog"), coneId,
//  leadId);
//
//  QVariant rootPhysId = addPhysVol(worldLogId, QVariant()); // no parent
//  QVariant childPhysId = addPhysVol(toyLogId, rootPhysId);
//}

bool GMDBManager::storeRootVolume(const unsigned int& id,
                                  const std::string& nodeType) {
    checkIsDBOpen();

    std::string tableName = "RootVolume";
    const unsigned int typeId = getTableIdFromNodeType(nodeType);
    std::vector<std::string> cols = getTableColumnNames(tableName);

    // set the SQL query
    std::string queryStr;
    std::string sql = fmt::format("INSERT INTO {0} ({1}, {2}) VALUES (?, ?)",
                                  tableName, cols[1], cols[2]);
    // preparing the SQL query
    sqlite3_stmt* st = nullptr;
    int rc = -1;
    rc = sqlite3_prepare_v2(m_d->m_dbSqlite, sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) {
        printf(
            "[SQLite ERR] (%s) : Error msg: %s\n", __func__,
            sqlite3_errmsg(m_d->m_dbSqlite));  // TODO: add __func__ to all
                                               // error messages, as I did here
        exit(EXIT_FAILURE);
    }
    if (m_debug) std::cout << "Query string:" << sql << std::endl;  // debug
    // bind the parameters
    rc = sqlite3_bind_int(st, 1, id);
    rc = sqlite3_bind_int(st, 2, typeId);
    // execute the query
    rc = sqlite3_step(st);
    if (rc != SQLITE_DONE) {
        printf("[SQLite ERR] (%s) : Error msg: %s\n", __func__,
               sqlite3_errmsg(m_d->m_dbSqlite));
        exit(EXIT_FAILURE);
    }
    // finalize
    sqlite3_finalize(st);

    if (rc != 0) {
        return false;
    }
    return true;
}

std::vector<std::string> GMDBManager::getRootPhysVol() {
    // get the ID of the ROOT vol from the table "RootVolume"
    sqlite3_stmt* stmt = m_d->selectAllFromTable("RootVolume");
    // declare the data we want to fetch
    unsigned int id = 0;
    unsigned int typeId = 0;
    // execute the statement on all rows
    int rc = -1;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // get the data
        id = sqlite3_column_int(stmt, 0);
        typeId = sqlite3_column_int(stmt, 1);
        // TODO: fill a cache
    }
    if (rc != SQLITE_DONE) {
        std::string errmsg(sqlite3_errmsg(m_d->m_dbSqlite));
        sqlite3_finalize(stmt);
        throw errmsg;
    }
    // finalize
    sqlite3_finalize(stmt);

    return getItemAndType(typeId, id);
}

std::string GMDBManager::getDBFilePath() { return m_dbpath; }

// FIXME: TODO: move to an utility class
int lastIndexOf(std::vector<std::string> v, std::string str, int pos = 0) {
    auto it = std::find(std::next(v.rbegin(), v.size() - pos), v.rend(), str);
    if (it != v.rend()) {
        auto idx = std::distance(v.begin(), it.base() - 1);
        //    std::cout << idx << std::endl;
        return idx;
    }
    return -1;  // no item matched
}

int GMDBManager::getTableColIndex(const std::string& tableName,
                                  const std::string& colName) {
    std::vector<std::string> colFields = m_tableNames.at(tableName);
    return lastIndexOf(colFields, colName);
}
