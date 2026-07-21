
Clone and build

git clone https://github.com/octobotics/i2w.git
cd  i2w 
mkdir build
 cd build
 cmake ..
 make 
sudo make install 


i2w::LifecycleResult

i2w::Severity::Warning
i2w::Severity::Fatal
i2w::Severity::Warning

i2w::PublisherOptions


i2w_clt

Command lIne tool to use i2w 

pub

./i2w pub local -r {int 2, string "sk", bool true, float 32.3, double 92}
./i2w sub local -r {int 2, string "sk", bool true, float 32.3, double 92} /demo


./i2w pub network -r {int 2, string "sk", bool true, float 32.3, double 92}
./i2w sub network -r {int 2, string "sk", bool true, float 32.3, double 92}