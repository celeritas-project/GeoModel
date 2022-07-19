#include "configregions.h"
#include "fsl_mainwindow.h"
#include "ui_fsl_mainwindow.h"
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QFontDatabase>
#include <QFileInfo>
#include <thread>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <cmath>
#include <unistd.h>
#include <libgen.h>
FSLMainWindow::FSLMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FSLMainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("FullSimLight-GUI (beta version)");

    //Setting up Models
    sens_det_model = new QStringListModel(this);
    g4ui_model = new QStringListModel(this);
  //  shape_model = new QStringListModel(this);
  //  ui->shape_view->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->sens_det_view->setModel(sens_det_model);
    ui->g4ui_view->setModel(g4ui_model);
  //  ui->shape_view->setModel(shape_model);
    ui->sens_det_view->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->g4ui_view->setEditTriggers(QAbstractItemView::DoubleClicked);


    //Setting up the Regions Display
    region = new ConfigRegions(this);
    region_model = new QStandardItemModel(this);
    region_horizontalHeader.append("Region Name");
    region_horizontalHeader.append("RootLV Names");
    region_horizontalHeader.append("Electron Cut (GeV)");
    region_horizontalHeader.append("Proton Cut (GeV)");
    region_horizontalHeader.append("Positron Cut (GeV)");
    region_horizontalHeader.append("Gamma Cut (GeV)");
    region_model->setHorizontalHeaderLabels(region_horizontalHeader);
    ui->regions_table->setModel(region_model);
    ui->regions_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->regions_table->resizeRowsToContents();
    ui->regions_table->resizeColumnsToContents();


    //Setting up the User Actions Display
    user_action_model = new QStandardItemModel(this);
   // user_action_horizontalHeader.append("Type of Action");
    user_action_horizontalHeader.append("File");
    user_action_model->setHorizontalHeaderLabels(user_action_horizontalHeader);
    ui->user_action_table->setModel(user_action_model);
    ui->user_action_table->horizontalHeader()->setStretchLastSection(true);
    ui->user_action_table->resizeRowsToContents();
    ui->user_action_table->resizeColumnsToContents();


    //Setting up Connections
    connect(ui->pB_geom, &QPushButton::released, this, &FSLMainWindow::assign_geom_file);
    connect(ui->actionSave, &QAction::triggered, this, &FSLMainWindow::save_configuration);
    connect(ui->actionSave_as, &QAction::triggered, this, &FSLMainWindow::save_configuration_as);
    connect(ui->actionOpen, &QAction::triggered, this, &FSLMainWindow::load_configuration);
    connect(ui->pB_view, &QPushButton::released, this, &FSLMainWindow::view_configuration);
    connect(ui->pB_Run, &QPushButton::released, this, &FSLMainWindow::run_configuration);
    connect(ui->pB_gmex, &QPushButton::released, this, &FSLMainWindow::run_gmex);
    connect(ui->pB_gmclash, &QPushButton::released, this, &FSLMainWindow::run_gmclash);
    connect(ui->pB_main_clear, &QPushButton::released, this, &FSLMainWindow::clear_main_status);
    connect(ui->pB_pythia_browse, &QPushButton::released, this, &FSLMainWindow::assign_pythia_file);
    connect(ui->pB_magnetic_field_plugin, &QPushButton::released, this, &FSLMainWindow::assign_magnetic_field_plugin_file);
    connect(ui->pB_magnetic_field_map, &QPushButton::released, this, &FSLMainWindow::assign_magnetic_field_map);

    connect(ui->pB_add_sens_det, &QPushButton::released, this, &FSLMainWindow::add_sens_det);
    connect(ui->pB_del_sens_det, &QPushButton::released, this, &FSLMainWindow::del_sens_det);
    connect(ui->pB_add_region, &QPushButton::released, this, &FSLMainWindow::pop_up_regions);
    connect(ui->pB_del_region, &QPushButton::released, this, &FSLMainWindow::del_region);
    connect(ui->pB_add_g4ui, &QPushButton::released, this, &FSLMainWindow::add_g4ui);
    connect(ui->pB_del_g4ui, &QPushButton::released, this, &FSLMainWindow::del_g4ui);
   // connect(ui->pB_add_shape_ext_file, &QPushButton::released, this, &FSLMainWindow::add_shape_ext);
   // connect(ui->pB_del_shape_ext_file, &QPushButton::released, this, &FSLMainWindow::del_shape_ext);


  //  connect(ui->pB_run_actions, &QPushButton::released, this, &FSLMainWindow::assign_run_actions_file);
  //  connect(ui->pB_event_actions, &QPushButton::released, this, &FSLMainWindow::assign_event_actions_file);
  //  connect(ui->pB_stepping_actions, &QPushButton::released, this, &FSLMainWindow::assign_stepping_actions_file);
  //  connect(ui->pB_stacking_actions, &QPushButton::released, this, &FSLMainWindow::assign_stacking_actions_file);
  //  connect(ui->pB_tracking_actions, &QPushButton::released, this, &FSLMainWindow::assign_tracking_actions_file);
    connect(ui->pB_add_user_action, &QPushButton::released, this, &FSLMainWindow::add_user_action);
    connect(ui->pB_del_user_action, &QPushButton::released, this, &FSLMainWindow::del_user_action);

    connect(ui->actionQuit, &QAction::triggered, qApp, &QApplication::quit);
    

    //Setting widget properties
    ui->sB_NOE->setMaximum(10000);
    ui->sB_NOT->setMaximum(std::thread::hardware_concurrency());
    ui->sB_control->setMaximum(5);
    ui->sB_run->setMaximum(5);
    ui->sB_event->setMaximum(5);
    ui->sB_tracking->setMaximum(5);
    ui->cB_gen_options->setCurrentIndex(0);
    ui->cB_particle->setCurrentIndex(0);
    ui->pB_pythia_browse->setEnabled(false);
    ui->cB_pythia_type_of_eve->setEnabled(false);
    ui->pB_magnetic_field_map->setEnabled(false);
    ui->pB_magnetic_field_plugin->setEnabled(false);
    ui->cB_particle->setCurrentIndex(0);
    ui->lE_px->setText("0");
    ui->lE_py->setText("10");
    ui->lE_pz->setText("0");

    ui->lE_PLN->setText("FTFP_BERT");
    ui->lE_fixed_MF->setText("4.0");
    ui->sB_NOT->setValue(std::thread::hardware_concurrency());
    ui->sB_NOE->setValue(10);
    number_of_primaries_per_event = 1;
    ui->lE_hits->setText("HITS.root");
    ui->lE_histo->setText("HISTO.root");
    ui->tB_view_config->setCurrentFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    ui->tB_view_config->setFontPointSize(13);

    p_x_validator = new QDoubleValidator(-100000.0,100000.0,8);
    p_y_validator = new QDoubleValidator(-100000.0,100000.0,8);
    p_z_validator = new QDoubleValidator(-100000.0,100000.0,8);
    mag_field_validator = new QDoubleValidator(-100000.0,100000.0,8);
    
    lo = QLocale::C;
    lo.setNumberOptions(QLocale::RejectGroupSeparator);
    
    p_x_validator->setLocale(lo);
    p_y_validator->setLocale(lo);
    p_z_validator->setLocale(lo);
    mag_field_validator->setLocale(lo);
    
    ui->lE_px->setValidator(p_x_validator);
    ui->lE_py->setValidator(p_y_validator);
    ui->lE_pz->setValidator(p_z_validator);
    ui->lE_fixed_MF->setValidator(mag_field_validator);

    ui->lE_hits->setEnabled(false);
    ui->lE_histo->setEnabled(false);


  //  ui->tab->setEnabled(false);//Shape tab (Change name on UI)
   // ui->Region->setEnabled(false);
   // ui->User_Actions->setEnabled(false);



    //Setting up Signals
    connect(ui->cB_gen_options, QOverload<int>::of(&QComboBox::currentIndexChanged), this ,&FSLMainWindow::configure_generator);
    connect(ui->cB_magnetic_field, QOverload<int>::of(&QComboBox::currentIndexChanged), this ,&FSLMainWindow::configure_magnetic_field);
    connect(ui->cB_pythia_type_of_eve, QOverload<int>::of(&QComboBox::currentIndexChanged), this ,&FSLMainWindow::check_if_pythia_file);

    connect(this, &FSLMainWindow::send_error_message, this, &FSLMainWindow::catch_error_message);
    connect(ui->sens_det_view, SIGNAL(clicked(QModelIndex)), this, SLOT(get_sens_det_index(QModelIndex)));
    connect(ui->g4ui_view, SIGNAL(clicked(QModelIndex)), this, SLOT(get_g4ui_index(QModelIndex)));
  //  connect(ui->shape_view, SIGNAL(clicked(QModelIndex)), this, SLOT(get_shape_index(QModelIndex)));
    connect(region,&ConfigRegions::send_config,this,&FSLMainWindow::add_region);
    connect(&fullSimLight_process,SIGNAL(readyReadStandardOutput()),this,SLOT(fsmlreadyReadStandardOutput()));
    connect(&fullSimLight_process,SIGNAL(readyReadStandardError()),this,SLOT(fsmlreadyReadStandardError()));

    connect(&gmex_process,SIGNAL(readyReadStandardOutput()),this,SLOT(gmexreadyReadStandardOutput()));
    connect(&gmex_process,SIGNAL(readyReadStandardError()),this,SLOT(gmexreadyReadStandardError()));

    connect(&fullSimLight_process,SIGNAL(started()),this,SLOT(fsml_process_started()));
    connect(&fullSimLight_process,SIGNAL(finished(int , QProcess::ExitStatus )),this,SLOT(fsml_process_finished()));


}

FSLMainWindow::~FSLMainWindow()
{
    delete ui;
    delete sens_det_model;
    delete g4ui_model;
    delete region;
    delete region_model;
    delete user_action_model;
   // delete shape_model;
    delete p_x_validator;
    delete p_y_validator;
    delete p_z_validator;
    delete mag_field_validator;

}


//Custom Signal to append to status bars
void FSLMainWindow::catch_error_message(std::string info)
{
    std::cout << info << std::endl;
}

//Get index of the row in the sensitive detector extensions display when clicked
void FSLMainWindow::get_sens_det_index(QModelIndex sens_det_index)
{
    sens_det_number = sens_det_index.row();
}

//Add the Sensitive detector file
void FSLMainWindow::add_sens_det()
{
    std::string sens_det_file = this->get_file_name();
    if(sens_det_file.find(".dylib") != std::string::npos || sens_det_file.find(".so") != std::string::npos)
    {QString q_sens_det_file = QString::fromUtf8(sens_det_file.c_str());
    sens_det_model->insertRow(sens_det_model->rowCount());
    QModelIndex sens_det_index = sens_det_model->index(sens_det_model->rowCount()-1);
    sens_det_model->setData(sens_det_index, q_sens_det_file);
    }
}


//Delete the sensitive_detector_ext
void FSLMainWindow::del_sens_det()
{
    sens_det_model->removeRow(sens_det_number);
}


//Add the sensitive detectors to the sensitive detector extensions list.
void FSLMainWindow::configure_sens_det_actions()
{
    sensitive_detector_extensions.clear();
    for(int i = 0; i<=ui->sens_det_view->model()->rowCount()-1; i++)
    {
        sensitive_detector_extensions.push_back((sens_det_model->
                           index( i, 0 ).data( Qt::DisplayRole ).toString()).toStdString());

    }
}


//Get index of the row in the Shape extensions display when clicked
/*void FSLMainWindow::get_shape_index(QModelIndex shape_index)
{
    shape_number = shape_index.row();
}

//Add a shape extension
void FSLMainWindow::add_shape_ext()
{
    std::string shape_ext_file = this->get_file_name();
    if(shape_ext_file.find(".dylib") != std::string::npos || shape_ext_file.find(".so") != std::string::npos)
    {QString q_shape_ext_file = QString::fromUtf8(shape_ext_file.c_str());
    shape_model->insertRow(shape_model->rowCount());
    QModelIndex shape_index = shape_model->index(shape_model->rowCount()-1);
    shape_model->setData(shape_index, q_shape_ext_file);
    }
}

//Delete the shape extension
void FSLMainWindow::del_shape_ext()
{
    shape_model->removeRow(shape_number);
}

//Add the shape extenions to the shape extensions list.
void FSLMainWindow::configure_shape_ext()
{
    shape_extensions.clear();
    for(int i = 0; i<=ui->shape_view->model()->rowCount()-1; i++)
    {
        shape_extensions.push_back((shape_model->
                           index( i, 0 ).data( Qt::DisplayRole ).toString()).toStdString());

    }
}*/


//Get index of row in g4ui display when clicked
void FSLMainWindow::get_g4ui_index(QModelIndex sens_det_index)
{
    g4ui_number = sens_det_index.row();
}

//Add a g4ui command
void FSLMainWindow::add_g4ui()
{
    QString g4ui_comm = ui->lE_g4ui->text();
    if(g4ui_comm!="")
    {
    g4ui_model->insertRow(g4ui_model->rowCount());
    QModelIndex g4ui_index = g4ui_model->index(g4ui_model->rowCount()-1);
    g4ui_model->setData(g4ui_index, g4ui_comm);
    ui->lE_g4ui->clear();
    }

}

//Delete a g4ui command
void FSLMainWindow::del_g4ui()
{
    g4ui_model->removeRow(g4ui_number);
}

//Add g4ui commands to g4ui commands list
void FSLMainWindow::configure_g4ui_command()
{

    g4ui_commands.clear();

    if(ui->cB_control->isChecked())
    {
        std::string control_verbosity = (ui->sB_control->text()).toStdString();
        std::string control_command = "/control/verbose " + control_verbosity;
        g4ui_commands.push_back(control_command);
    }

    if(ui->cB_run->isChecked())
    {
        std::string run_verbosity = (ui->sB_run->text()).toStdString();
        std::string run_command = "/run/verbose " + run_verbosity;
        g4ui_commands.push_back(run_command);
    }

    if(ui->cB_event->isChecked())
    {
        std::string event_verbosity = (ui->sB_event->text()).toStdString();
        std::string event_command = "/event/verbose " + event_verbosity;
        g4ui_commands.push_back(event_command);
    }

    if(ui->cB_tracking->isChecked())
    {
        std::string tracking_verbosity = (ui->sB_tracking->text()).toStdString();
        std::string tracking_command = "/tracking/verbose " + tracking_verbosity;
        g4ui_commands.push_back(tracking_command);
    }

    for(int row = 0; row < ui->g4ui_view->model()->rowCount(); ++row)
    {
        std::string comm = (ui->g4ui_view->model()->index(row,0).data().toString()).toStdString();
        if(comm != ""){
        g4ui_commands.push_back(comm);
    }
    }

    g4ui_commands.push_back("/control/cout/prefixString G4Worker_");
    g4ui_commands.push_back("/run/numberOfThreads " + ui->sB_NOT->text().toStdString());

    if(magnetic_field_type=="Fixed Axial")
    {
    g4ui_commands.push_back("/FSLdet/setField " + ui->lE_fixed_MF->text().toStdString() +" tesla");
    }

    if(generator=="Pythia")
    {
    g4ui_commands.push_back("/run/initialize");
    }

    if(generator=="Particle Gun")
    {
    g4ui_commands.push_back("/FSLgun/primaryPerEvt " + std::to_string(number_of_primaries_per_event));
    g4ui_commands.push_back("/FSLgun/energy  " + particle_energy);
    g4ui_commands.push_back("/FSLgun/particle  " + particle);
    g4ui_commands.push_back("/FSLgun/direction  " + particle_direction);

    }

   // g4ui_commands.push_back("/process/list");



}


//Pop up add regions window when + clicked
void FSLMainWindow::pop_up_regions()
{
    region->show();
}

//Add a Region
void FSLMainWindow::add_region(std::string  region_name, std::string frootLV_names
                               ,double electron_cut , double proton_cut
                               ,double positron_cut , double gamma_cut)
{

    int rows = ui->regions_table->model()->rowCount();
    ui->regions_table->model()->insertRows(rows,1);

    QString q_region_name = QString::fromUtf8(region_name.c_str());
    QString q_frootlv_names = QString::fromUtf8(frootLV_names.c_str());

    ui->regions_table->model()->setData(ui->regions_table->model()->index(rows,0),q_region_name);
    ui->regions_table->model()->setData(ui->regions_table->model()->index(rows,1),q_frootlv_names);
    ui->regions_table->model()->setData(ui->regions_table->model()->index(rows,2),electron_cut);
    ui->regions_table->model()->setData(ui->regions_table->model()->index(rows,3),proton_cut);
    ui->regions_table->model()->setData(ui->regions_table->model()->index(rows,4),positron_cut);
    ui->regions_table->model()->setData(ui->regions_table->model()->index(rows,5),gamma_cut);


}

//Delete a Region
void FSLMainWindow::del_region()
{
    QModelIndexList indexes =  ui->regions_table->selectionModel()->selectedRows();
    int countRow = indexes.count();

    for( int i = countRow; i > 0; i--)
           region_model->removeRow( indexes.at(i-1).row(), QModelIndex());
}



//Add regions to regions list (struct).
void FSLMainWindow::configure_regions()
{
    regions.clear();
    for(int row = 0 ; row < ui->regions_table->model()->rowCount(); ++row )
    {
        std::string reg_name = ((ui->regions_table->model()->index(row,0)).data().toString()).toStdString();
        std::vector<std::string> froot_names = this->parse_froot_string(((ui->regions_table->model()
                                                                          ->index(row,1)).data().toString()).toStdString());

        double ele_cut = std::stod((ui->regions_table->model()->index(row,2)).data().toString().toStdString());
        double prot_cut = std::stod((ui->regions_table->model()->index(row,3)).data().toString().toStdString());
        double posit_cut = std::stod((ui->regions_table->model()->index(row,4)).data().toString().toStdString());
        double gam_cut = std::stod((ui->regions_table->model()->index(row,5)).data().toString().toStdString());

        Region region_from_list = {reg_name, froot_names, ele_cut,prot_cut,posit_cut,gam_cut};
        regions.push_back(region_from_list);

    }
}

//Function to turn string of fRootLV names into a vector.
std::vector<std::string> FSLMainWindow::parse_froot_string(std::string input_froot_string)
{
    std::vector<std::string> frootLVNames;
    std::stringstream ss( input_froot_string );

    while( ss.good() )
    {
        std::string substr;
        getline( ss, substr, ',' );
        if(substr!=""){
        frootLVNames.push_back( substr );
        }
    }

    return frootLVNames;

}

/*
//Function to add a Run actions file
void FSLMainWindow::assign_run_actions_file()
{
    QString q_run_file_name =  QString::fromUtf8((this->get_file_name()).c_str());

    if(q_run_file_name!="")
    {
    int rows = ui->user_action_table->model()->rowCount();
    ui->user_action_table->model()->insertRows(rows,1);
    ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,0),"Run");
    ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,1),q_run_file_name);
    }
}

//Function to add a Event actions file
void FSLMainWindow::assign_event_actions_file()
{
    QString q_event_file_name =  QString::fromUtf8((this->get_file_name()).c_str());

    if(q_event_file_name!="")
    {
    int rows = ui->user_action_table->model()->rowCount();
    ui->user_action_table->model()->insertRows(rows,1);
    ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,0),"Event");
    ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,1),q_event_file_name);
    }
}

//Function to add a Stepping actions file
void FSLMainWindow::assign_stepping_actions_file()
{
    QString q_stepping_file_name =  QString::fromUtf8((this->get_file_name()).c_str());

    if(q_stepping_file_name!="")
    {
    int rows = ui->user_action_table->model()->rowCount();
    ui->user_action_table->model()->insertRows(rows,1);
    ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,0),"Stepping");
    ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,1),q_stepping_file_name);
    }
}

//Function to add a Stacking actions file
void FSLMainWindow::assign_stacking_actions_file()
{
    QString q_stacking_file_name =  QString::fromUtf8((this->get_file_name()).c_str());

    if(q_stacking_file_name!="")
    {
    int rows = ui->user_action_table->model()->rowCount();
    ui->user_action_table->model()->insertRows(rows,1);
    ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,0),"Stacking");
    ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,1),q_stacking_file_name);
    }
}

//Function to add a Tracking actions file
void FSLMainWindow::assign_tracking_actions_file()
{
    QString q_tracking_file_name =  QString::fromUtf8((this->get_file_name()).c_str());

    if(q_tracking_file_name!="")
    {
    int rows = ui->user_action_table->model()->rowCount();
    ui->user_action_table->model()->insertRows(rows,1);
    ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,0),"Tracking");
    ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,1),q_tracking_file_name);
    }
}
*/

//Function to add a user action file
void FSLMainWindow::add_user_action()
{
    QString q_user_file_name =  QString::fromUtf8((this->get_file_name()).c_str());

    if(q_user_file_name!="")
    {
    int rows = ui->user_action_table->model()->rowCount();
    ui->user_action_table->model()->insertRows(rows,1);
    ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,0),q_user_file_name);
    }
} 

//Function to add a delete a user actions file
void FSLMainWindow::del_user_action()
{
    QModelIndexList user_action_indexes =  ui->user_action_table->selectionModel()->selectedRows();
    int countRow = user_action_indexes.count();

    for( int i = countRow; i > 0; i--)
           user_action_model->removeRow( user_action_indexes.at(i-1).row(), QModelIndex());
}


//Function to add user actions to respective lists.
void FSLMainWindow::configure_actions()
{
   // run_actions.clear();
   // event_actions.clear();
   // stepping_actions.clear();
   // stacking_actions.clear();
   // tracking_actions.clear();
    user_action_extensions.clear();

    for(int row = 0 ; row < ui->user_action_table->model()->rowCount(); ++row )
    {
      //  std::string type_of_action = ((ui->user_action_table->model()->index(row,0)).data().toString()).toStdString();
        std::string associated_file = ((ui->user_action_table->model()->index(row,0)).data().toString()).toStdString();

      /*  if(type_of_action == "Run")
        {
            run_actions.push_back(associated_file);
        }

        else if(type_of_action == "Event")
        {
            event_actions.push_back(associated_file);
        }

        else if(type_of_action == "Stepping")
        {
            stepping_actions.push_back(associated_file);
        }

        else if(type_of_action == "Stacking")
        {
            stacking_actions.push_back(associated_file);
        }

        else if(type_of_action == "Tracking")
        {
            tracking_actions.push_back(associated_file);
        }
        */

        user_action_extensions.push_back(associated_file);

    }


}


//Clear the view/run configuration window
void FSLMainWindow::clear_main_status()
{
  ui->tB_view_config->clear();
}

//Function to allow user to select a file
std::string FSLMainWindow::get_file_name()
{

      QString filename= QFileDialog::getOpenFileName(this, "Choose File");

       return filename.toStdString();
}

//Function to allow user to select a directory
std::string FSLMainWindow::get_directory()
{

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"/home",
                                                    QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);

       return dir.toStdString();
}

//Function to select geometry file
void FSLMainWindow::assign_geom_file()
{



  if (geom_file_directory.empty()) geom_file_directory= (QDir::currentPath()).toStdString() +"/";
  QString fileName = QFileDialog::getOpenFileName(this,
						  tr("Select Geometry"), geom_file_directory.c_str(), tr("Geometry inputs (*.db *.gdml *.so *.dylib)"));
  
  if (fileName.isEmpty()) return;
  
  std::string   geometry_file=fileName.toStdString();
  std::string   geometry_base=basename(const_cast<char *> (geometry_file.c_str()));
  std::string   geometry_directory=dirname(const_cast<char *> (geometry_file.c_str()));
  geom_file_address=geometry_directory+"/"+geometry_base;
  
  // Store these values:
  geom_file_directory=geometry_directory;// When browser is reopened, start from here.
  //    QFileInfo file(QString::fromUtf8(load_file_name.c_str()));
  ui->le_GI->setText(geom_file_address.c_str());
  ui->le_GI->adjustSize();
  
  if(geometry_base.find(".db") != std::string::npos
     || geometry_base.find(".gdml") != std::string::npos
     || geometry_base.find(".dylib") != std::string::npos
     || geometry_base.find(".so") != std::string::npos)
    {
    }
  else
    {
      ui->tB_view_config->append("Supported Geometry file formats are .db,.gdml,.dylib,.so");
    }
}

//Function to select a pythia file
void FSLMainWindow::assign_pythia_file()
{
    pythia_input_file = this->get_file_name();
}

void FSLMainWindow::check_if_pythia_file()
{
    if(ui->cB_pythia_type_of_eve->currentIndex()==3)
    {
        ui->pB_pythia_browse->setEnabled(true);
    }
    else
    {
        ui->pB_pythia_browse->setEnabled(false);
    }
}

//Function to select a magnetic field plugin file
void FSLMainWindow::assign_magnetic_field_plugin_file()
{
    magnetic_field_plugin_file = this->get_file_name();
}

//Function to assign magnetic field map
void FSLMainWindow::assign_magnetic_field_map()
{
    magnetic_field_map = this->get_file_name();
}

//Function to configure particle energy and direction
void FSLMainWindow::configure_energy_direction()
{
    p_x = std::stod(ui->lE_px->text().toStdString());
    p_y = std::stod(ui->lE_py->text().toStdString());
    p_z = std::stod(ui->lE_pz->text().toStdString());
    double p = sqrt(pow(p_x,2)+pow(p_y,2)+pow(p_z,2));

    x_dir = std::to_string(p_x/p);
    y_dir = std::to_string(p_y/p);
    z_dir = std::to_string(p_z/p);

    particle_direction = x_dir + " " + y_dir + " " + z_dir;

    particle_energy = std::to_string(p) + " GeV";




}

//Function to select type of generator
void FSLMainWindow::configure_generator()
{
    generator = (ui->cB_gen_options->currentText()).toStdString();

    if(generator=="Particle Gun")
    {
        this->configure_energy_direction();
        ui->pB_pythia_browse->setEnabled(false);
        ui->cB_pythia_type_of_eve->setEnabled(false);
        ui->cB_particle->setEnabled(true);
        ui->lE_px->setEnabled(true);
        ui->lE_py->setEnabled(true);
        ui->lE_pz->setEnabled(true);


        particle = (ui->cB_particle->currentText()).toStdString();
        pythia_type_of_event = "";
        pythia_input_file = "";
    }

    else if(generator=="Pythia")
    {
        ui->cB_pythia_type_of_eve->setEnabled(true);

        ui->cB_particle->setEnabled(false);
        ui->lE_px->setEnabled(false);
        ui->lE_py->setEnabled(false);
        ui->lE_pz->setEnabled(false);

        if(ui->cB_pythia_type_of_eve->currentIndex()==3)
        {
        pythia_type_of_event = "";
        ui->pB_pythia_browse->setEnabled(true);

        }

        else
        {
        pythia_input_file = "";
        pythia_type_of_event = (ui->cB_pythia_type_of_eve->currentText()).toStdString();

        }


        particle = "";
        particle_energy = "";
        particle_direction = "";
        p_x = 0;
        p_y = 0;
        p_z = 0;

    }

}

void FSLMainWindow::configure_magnetic_field()
{
    magnetic_field_type = (ui->cB_magnetic_field->currentText()).toStdString();

    if(magnetic_field_type == "Fixed Axial")
    {
        magnetic_field = (ui->lE_fixed_MF->text()).toStdString();
        magnetic_field_plugin_file = "";
        magnetic_field_map = "";
        ui->pB_magnetic_field_map->setEnabled(false);
        ui->pB_magnetic_field_plugin->setEnabled(false);

        ui->lE_fixed_MF->setEnabled(true);



    }

    else
    {
      //  magnetic_field_map = (ui->lE_magnetic_field_map->text()).toStdString();
        magnetic_field = "";
        ui->lE_fixed_MF->setEnabled(false);

        ui->pB_magnetic_field_map->setEnabled(true);
        ui->pB_magnetic_field_plugin->setEnabled(true);
    }
}

//Function to display configuration properly (Break up string)
std::vector<std::string> FSLMainWindow::display_configuration(const std::string &s)
{
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;
    char delim = ',';

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;

}

//Function to view current configuration
void FSLMainWindow::view_configuration()
{
    ui->tB_view_config->clear();
    this->create_configuration();
    const auto s = j.dump();
    std::vector<std::string> display_vector = this->display_configuration(s);

    for(auto &i:display_vector)
    {
        QString i_str = QString::fromUtf8(i.c_str());
        ui->tB_view_config->append(i_str);
    }
}

//Function to save current configuration
void FSLMainWindow::save_configuration()
{

    create_configuration();
    if (config_file_name.empty()) {
      save_configuration_as();
      return;
    }
    //    config_file_name = (ui->lE_CFN->text()).toStdString();
    std::ofstream o(config_file_name);
    o << std::setw(4) << j << std::endl;
}

//Function to save current configuration
void FSLMainWindow::save_configuration_as()
{
    create_configuration();
    if (save_directory.empty()) save_directory= (QDir::currentPath()).toStdString() +"/";
    
    QString fileName = QFileDialog::getSaveFileName(this,
						    tr("Save Configuration"), save_directory.c_str(), tr("Configuration Files (*.json)"));
    if (fileName.isEmpty()) return;
    std::string   save_file=fileName.toStdString();
    std::string   save_base=basename(const_cast<char *> (save_file.c_str()));
    save_directory=dirname(const_cast<char *> (save_file.c_str()));

    std::ofstream o(save_directory+"/"+save_base);
    o << std::setw(4) << j << std::endl;

    
    config_file_name=save_directory+"/"+save_base;
    ui->lE_CFN->setText(("Config file: " + config_file_name).c_str());
    ui->lE_CFN->adjustSize();
 
    


}

//Function to run a selected configuration.
void FSLMainWindow::run_configuration()
{

  if (geom_file_address.empty()) {
    QMessageBox::information(this, "Info", "First Select Geometry input");
     return;
  }
  create_configuration();
  
  std::string tmpConf="/tmp/fslconfig-"+std::to_string(getuid())+"-"+std::to_string(getpid())+".json";
  
  std::ofstream o(tmpConf);
  o << std::setw(4) << j << std::endl;

  {
    QString Command;    //Contains the command to be executed
    QStringList args;   //Contains arguments of the command
    
    //Needs to be fixed. Should not be a hard coded path.
    Command = "fullSimLight";
    
    args<<"-c"<< QString::fromUtf8(tmpConf.c_str());
    fullSimLight_process.start(Command, args, QIODevice::ReadOnly);
  }

}

//Function to run a selected configuration.
void FSLMainWindow::run_gmex()
{

  if (geom_file_address.empty()) {
    QMessageBox::information(this, "Info", "First Select Geometry input");
     return;
  }
  {
    QString Command;    //Contains the command to be executed
    QStringList args;   //Contains arguments of the command
    
    //Needs to be fixed. Should not be a hard coded path.
    Command = "gmex";
     
    args << QString::fromUtf8(geom_file_address.c_str());
    gmex_process.start(Command, args, QIODevice::ReadOnly);
  }

}

//Function to run a selected configuration.
void FSLMainWindow::run_gmclash()
{

  if (geom_file_address.empty()) {
    QMessageBox::information(this, "Info", "First Select Geometry input");
     return;
  }
  {
    QString Command;    //Contains the command to be executed
    QStringList args;   //Contains arguments of the command
    
    //Needs to be fixed. Should not be a hard coded path.
    Command = "gmclash";
     
    args << QString("-g") <<  QString::fromUtf8(geom_file_address.c_str());
    gmex_process.start(Command, args, QIODevice::ReadOnly);
  }

}


//Function to blur out Buttons when fsml QProcess started
void FSLMainWindow::fsml_process_started()
{
    ui->pB_gmex->setEnabled(false);
    ui->pB_gmclash->setEnabled(false);
    ui->pB_Run->setEnabled(false);

}

//Function to reactivate Buttons when fsml QProcess stopped
void FSLMainWindow::fsml_process_finished()
{
    ui->pB_gmclash->setEnabled(true);
    ui->pB_gmex->setEnabled(true);
    ui->pB_Run->setEnabled(true);

}


//Function to get output from fsml run process
void FSLMainWindow::fsmlreadyReadStandardOutput()
{
    QString StdOut = fullSimLight_process.readAllStandardOutput(); //Reads standard output
    ui->tB_view_config->append(StdOut);
}

//Function to get error output from  fsml run process
void FSLMainWindow::fsmlreadyReadStandardError()
{
    QString StdErr = fullSimLight_process.readAllStandardError(); //Reads standard error output
    ui->tB_view_config->append(StdErr);
}


//Function to get output from gmex process
void FSLMainWindow::gmexreadyReadStandardOutput()
{
    QString StdOut = gmex_process.readAllStandardOutput(); //Reads standard output
    ui->tB_view_config->append(StdOut);
}

//Function to get error output from gmex process
void FSLMainWindow::gmexreadyReadStandardError()
{
    QString StdErr = gmex_process.readAllStandardError(); //Reads standard error output
    ui->tB_view_config->append(StdErr);
}



//Function to load configuration
void FSLMainWindow::load_configuration()
{

    if (save_directory.empty()) save_directory= (QDir::currentPath()).toStdString() +"/";
    QString fileName = QFileDialog::getOpenFileName(this,
						    tr("Open Configuration"), save_directory.c_str(), tr("Configuration Files (*.json)"));

    if (fileName.isEmpty()) return;

    std::string   load_file=fileName.toStdString();
    std::string   load_base=basename(const_cast<char *> (load_file.c_str()));
    std::string   load_directory=dirname(const_cast<char *> (load_file.c_str()));
    load_file_name=load_directory+"/"+load_base;

    // Store these values:
    save_directory=load_directory;// When browser is reopened, start from here.
    config_file_name=load_file_name;

    if(load_file_name.find(".json") != std::string::npos)
    {
    std::ifstream ifs(load_file_name);
    auto j_load = nlohmann::json::parse(ifs);

    //    QFileInfo file(QString::fromUtf8(load_file_name.c_str()));
    ui->lE_CFN->setText(("Config file: " + config_file_name).c_str());
    ui->lE_CFN->adjustSize();
    
    geom_file_address = j_load["Geometry"];

    ui->le_GI->setText(geom_file_address.c_str());
    ui->le_GI->adjustSize();

    
    physics_list_name = j_load["Physics list name"];
    ui->lE_PLN->setText(QString::fromUtf8(physics_list_name.c_str()));

    number_of_threads = j_load["Number of threads"];
    ui->sB_NOT->setValue(number_of_threads);

    number_of_events = j_load["Number of events"];
    ui->sB_NOE->setValue(number_of_events);

    magnetic_field = j_load["Magnetic Field Intensity"];
    ui->lE_fixed_MF->setText(QString::fromUtf8(magnetic_field.c_str()));


    generator = j_load["Generator"];

    if(generator=="Particle Gun")
    {
        ui->cB_gen_options->setCurrentIndex(0);

        ui->cB_particle->setEnabled(true);
        ui->lE_px->setEnabled(true);
        ui->lE_py->setEnabled(true);
        ui->lE_pz->setEnabled(true);

        particle = j_load["Particle"];
        ui->cB_particle->setCurrentText(QString::fromUtf8(particle.c_str()));

        p_x = j_load["p_x"];
        p_y = j_load["p_y"];
        p_z = j_load["p_z"];

        ui->lE_px->setText(QString::number(p_x));
        ui->lE_py->setText(QString::number(p_y));
        ui->lE_pz->setText(QString::number(p_z));


        ui->cB_pythia_type_of_eve->setCurrentIndex(0);
        ui->pB_pythia_browse->setEnabled(false);
        ui->cB_pythia_type_of_eve->setEnabled(false);

    }

    else
    {

        ui->cB_pythia_type_of_eve->setEnabled(true);

        ui->cB_gen_options->setCurrentIndex(1);

        pythia_type_of_event = j_load["Type of event"];

        if(pythia_type_of_event != "")
        {
        ui->cB_pythia_type_of_eve->setCurrentText(QString::fromUtf8(pythia_type_of_event.c_str()));
        ui->pB_pythia_browse->setEnabled(false);

        }
        else 
        {
        pythia_input_file = j_load["Event input file"];
        ui->cB_pythia_type_of_eve->setCurrentIndex(3);
        ui->pB_pythia_browse->setEnabled(true);

        }



        ui->lE_px->clear();
        ui->lE_py->clear();
        ui->lE_pz->clear();


        ui->cB_particle->setEnabled(false);
        ui->lE_px->setEnabled(false);
        ui->lE_py->setEnabled(false);
        ui->lE_pz->setEnabled(false);

    }

    sens_det_model->removeRows(0,sens_det_model->rowCount());
    for(const auto& element : j_load["Sensitive Detector Extensions"] )
    {
        std::string ele = element;
        QString q_element = QString::fromUtf8(ele.c_str());
        sens_det_model->insertRow(sens_det_model->rowCount());
        QModelIndex sens_det_index = sens_det_model->index(sens_det_model->rowCount()-1);
        sens_det_model->setData(sens_det_index, q_element);
    }

    region_model->removeRows(0,region_model->rowCount());
    for(const auto& element: j_load["Regions data"]){
        Region r;
        from_json(element,r);

        int rows = ui->regions_table->model()->rowCount();
        ui->regions_table->model()->insertRows(rows,1);

        QString q_r_name = QString::fromUtf8(r.Region_name.c_str());
        std::string r_froot = "";

        for(const auto& element: r.fRootLVnames)
        {
            r_froot = r_froot+element+",";
        }

        if(r_froot!="")
        {
            r_froot.pop_back();
        }

        QString q_r_froot = QString::fromUtf8(r_froot.c_str());


        ui->regions_table->model()->setData(ui->regions_table->model()->index(rows,0),q_r_name);
        ui->regions_table->model()->setData(ui->regions_table->model()->index(rows,1),q_r_froot);
        ui->regions_table->model()->setData(ui->regions_table->model()->index(rows,2),r.electron_cut);
        ui->regions_table->model()->setData(ui->regions_table->model()->index(rows,3),r.proton_cut);
        ui->regions_table->model()->setData(ui->regions_table->model()->index(rows,4),r.positron_cut);
        ui->regions_table->model()->setData(ui->regions_table->model()->index(rows,5),r.gamma_cut);


    }

    magnetic_field_type = j_load["Magnetic Field Type"];

    if(magnetic_field_type == "Fixed Axial")
    {
        ui->lE_fixed_MF->setEnabled(true);
        ui->cB_magnetic_field->setCurrentIndex(0);
        magnetic_field = j_load["Magnetic Field Intensity"];
        ui->lE_fixed_MF->setText(QString::fromUtf8(magnetic_field.c_str()));


        magnetic_field_plugin_file = "";
      //  ui->lE_magnetic_field_map->clear();
      //  ui->lE_magnetic_field_map->setEnabled(false);
        magnetic_field_map = "";
        ui->pB_magnetic_field_map->setEnabled(false);
        ui->pB_magnetic_field_plugin->setEnabled(false);

    }

    else{
        ui->pB_magnetic_field_map->setEnabled(true);
        ui->pB_magnetic_field_plugin->setEnabled(true);
        ui->cB_magnetic_field->setCurrentIndex(1);
        magnetic_field_plugin_file = j_load["Magnetic Field Plugin"];
        magnetic_field_map = j_load["Magnetic Field Map"];
       // ui->lE_magnetic_field_map->setText(QString::fromUtf8(magnetic_field_map.c_str()));

        magnetic_field = "";
        ui->lE_fixed_MF->clear();
        ui->lE_fixed_MF->setEnabled(false);


    }

    g4ui_model->removeRows(0,g4ui_model->rowCount());
    ui->cB_control->setCheckState(Qt::Unchecked);
    ui->cB_event->setCheckState(Qt::Unchecked);
    ui->cB_run->setCheckState(Qt::Unchecked);
    ui->cB_tracking->setCheckState(Qt::Unchecked);
    ui->sB_control->setValue(0);
    ui->sB_event->setValue(0);
    ui->sB_run->setValue(0);
    ui->sB_tracking->setValue(0);

    for(const auto& element : j_load["g4ui_commands"])
    {
        std::string g4ui_comm = element;


        if(g4ui_comm.find("/run/verbose") != std::string::npos)
        {
            ui->cB_run->setCheckState(Qt::Checked);
            int verbosity = g4ui_comm.back() - '0';
            ui->sB_run->setValue(verbosity);

        }

        else if(g4ui_comm.find("/control/verbose") != std::string::npos)
        {
            ui->cB_control->setCheckState(Qt::Checked);
            int verbosity = g4ui_comm.back() - '0';
            ui->sB_control->setValue(verbosity);

        }

        else if(g4ui_comm.find("/event/verbose") != std::string::npos)
        {
            ui->cB_event->setCheckState(Qt::Checked);
            int verbosity = g4ui_comm.back() - '0';
            ui->sB_event->setValue(verbosity);

        }

        else if(g4ui_comm.find("/tracking/verbose") != std::string::npos)
        {
            ui->cB_tracking->setCheckState(Qt::Checked);
            int verbosity = g4ui_comm.back() - '0';
            ui->sB_tracking->setValue(verbosity);

        }

        else if(g4ui_comm.find("/FSLdet/setField") != std::string::npos
                || g4ui_comm.find("/FSLgun/primaryPerEvt") != std::string::npos
                || g4ui_comm.find("/FSLgun/energy") != std::string::npos
                || g4ui_comm.find("/FSLgun/particle") != std::string::npos
                || g4ui_comm.find("/FSLgun/direction") != std::string::npos
                || g4ui_comm.find("/run/numberOfThreads") != std::string::npos
                || g4ui_comm.find("/control/cout/prefixString G4Worker_") != std::string::npos
                || g4ui_comm.find("/process/list") != std::string::npos
                || g4ui_comm.find("/run/initialize") != std::string::npos
                || g4ui_comm.find("/run/beamOn") != std::string::npos)
        {

        }

        else
        {
            QString q_g4ui_comm = QString::fromUtf8(g4ui_comm.c_str());
            g4ui_model->insertRow(g4ui_model->rowCount());
            QModelIndex g4ui_index = g4ui_model->index(g4ui_model->rowCount()-1);
            g4ui_model->setData(g4ui_index, q_g4ui_comm);
        }


    }

   /* user_action_model->removeRows(0,user_action_model->rowCount());
    for(const auto& element : j_load["Run Actions"])
    {
        std::string run_file = element;
        QString q_run_file = QString::fromUtf8(run_file.c_str());
        int rows = ui->user_action_table->model()->rowCount();
        ui->user_action_table->model()->insertRows(rows,1);
        ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,0),"Run");
        ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,1),q_run_file);

    }

    for(const auto& element : j_load["Event Actions"])
    {
        std::string event_file = element;
        QString q_event_file = QString::fromUtf8(event_file.c_str());
        int rows = ui->user_action_table->model()->rowCount();
        ui->user_action_table->model()->insertRows(rows,1);
        ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,0),"Event");
        ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,1),q_event_file);

    }

    for(const auto& element : j_load["Stepping Actions"])
    {
        std::string stepping_file = element;
        QString q_stepping_file = QString::fromUtf8(stepping_file.c_str());
        int rows = ui->user_action_table->model()->rowCount();
        ui->user_action_table->model()->insertRows(rows,1);
        ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,0),"Stepping");
        ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,1),q_stepping_file);

    }

    for(const auto& element : j_load["Stacking Actions"])
    {
        std::string stacking_file = element;
        QString q_stacking_file = QString::fromUtf8(stacking_file.c_str());
        int rows = ui->user_action_table->model()->rowCount();
        ui->user_action_table->model()->insertRows(rows,1);
        ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,0),"Stacking");
        ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,1),q_stacking_file);

    }

    for(const auto& element : j_load["Tracking Actions"])
    {
        std::string tracking_file = element;
        QString q_tracking_file = QString::fromUtf8(tracking_file.c_str());
        int rows = ui->user_action_table->model()->rowCount();
        ui->user_action_table->model()->insertRows(rows,1);
        ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,0),"Tracking");
        ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,1),q_tracking_file);

    }
    */

   /* shape_model->removeRows(0,shape_model->rowCount());
    for(const auto& element : j_load["Shape Extensions"] )
    {
        std::string ele = element;
        QString q_element = QString::fromUtf8(ele.c_str());
        shape_model->insertRow(shape_model->rowCount());
        QModelIndex shape_index = shape_model->index(shape_model->rowCount()-1);
        shape_model->setData(shape_index, q_element);
    }*/
    

    user_action_model->removeRows(0,user_action_model->rowCount());
    for(const auto& element : j_load["User Action Extensions"])
    {
        std::string run_file = element;
        QString q_run_file = QString::fromUtf8(run_file.c_str());
        int rows = ui->user_action_table->model()->rowCount();
        ui->user_action_table->model()->insertRows(rows,1);
        ui->user_action_table->model()->setData(ui->user_action_table->model()->index(rows,0),q_run_file);

    }

}

}

//Function to create the configuration
void FSLMainWindow::create_configuration()
{
    j.clear();
    number_of_events = ui->sB_NOE->value();
    number_of_threads = ui->sB_NOT->value();
    physics_list_name = (ui->lE_PLN->text()).toStdString();
    hits_file = (ui->lE_hits->text()).toStdString();
    histo_file = (ui->lE_histo->text()).toStdString();

    j["Geometry"] = geom_file_address;
    j["Physics list name"] = physics_list_name;
    j["Number of threads"] = number_of_threads;
    j["Number of events"] = number_of_events;

    this->configure_generator();
    j["Generator"] = generator;
    j["Particle"] = particle;
    j["p_x"] = p_x;
    j["p_y"] = p_y;
    j["p_z"] = p_z;
    j["Particle energy"] = particle_energy;
    j["Particle direction"] = particle_direction;
    j["Event input file"] = pythia_input_file;
    j["Type of event"] = pythia_type_of_event;

    this->configure_sens_det_actions();
    j["Sensitive Detector Extensions"] = sensitive_detector_extensions;
    j["Output Hits file"] = hits_file;
    j["Output Histo file"] = histo_file;

    this->configure_magnetic_field();
    j["Magnetic Field Type"] = magnetic_field_type;
    j["Magnetic Field Intensity"] = magnetic_field;
    j["Magnetic Field Map"] = magnetic_field_map;
    j["Magnetic Field Plugin"] = magnetic_field_plugin_file;

    this->configure_actions();
  //  j["Run Actions"] = run_actions;
  //  j["Event Actions"] = event_actions;
  //  j["Stepping Actions"] = stepping_actions;
  //  j["Stacking Actions"] = stacking_actions;
  //  j["Tracking Actions"] = tracking_actions;

    j["User Action Extensions"] = user_action_extensions;

    this->configure_regions();
    j["Regions data"] = regions;

   // this->configure_shape_ext();
   // j["Shape Extensions"] = shape_extensions;

    this->configure_g4ui_command();
    j["g4ui_commands"] = g4ui_commands;
}












//emit send_error_message(filename.toStdString());
