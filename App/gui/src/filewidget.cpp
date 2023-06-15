#include "filewidget.hpp"
#include <QLabel>
#include <QGridLayout>
#include "settings.hpp"
#include "filesystem.hpp"
#include <string>

namespace GUI{

    FileWidget::FileWidget(MainWindow *main):
            QWidget(main){
        
        QGridLayout *layout = new QGridLayout;
        std::string libfile = OS::libPath + OS::libPrefix + Settings::net.libFile + OS::libSuffix;
        layout->addWidget(new QLabel(tr(libfile.c_str())));
        setLayout(layout);
    }

}