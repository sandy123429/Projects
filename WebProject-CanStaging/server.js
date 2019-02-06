var http = require('http');
var express = require('express');
var fs = require ('fs');
var app = express();
var server = http.createServer(app);
var bodyParser = require('body-parser')
var loki = require('lokijs')
var moment = require('moment')
var acc;
var db = new loki('user.db');
var users = db.addCollection('users');
var data = db.addCollection('data');
var dydata = data.addDynamicView('DynamicData');
var i18nn=require('i18n');
//app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'ejs');
app.use(bodyParser.urlencoded({extended: true}));
app.use(bodyParser.json());
app.use("/bootstrap_input",express.static(__dirname + "/bootstrap_input"));
app.use("/ResponsiveSlides",express.static(__dirname + "/ResponsiveSlides"));
app.use("/node_modules",express.static(__dirname + "/node_modules"));
app.use("/tnm_tables",express.static(__dirname + "/tnm_tables"));
app.use("/csv",express.static(__dirname + "/csv"));


var passport = require('passport');//---
var LocalStrategy = require('passport-local').Strategy;//---
var flash = require('connect-flash');//---
var cookieParser = require('cookie-parser');//---
var i18n = require('./i18n');//---


app.use(cookieParser());//---以下一直到get
app.use(i18n);
app.use(require('express-session')({
    secret: 'keyboard cat',
    resave: false,
    saveUninitialized: false
}));
app.use(passport.initialize());
app.use(flash());
app.use(passport.session());
// Session-persisted message middleware

app.use(function(req, res, next){
  var err = req.session.error,
      msg = req.session.notice,
      success = req.session.success;

  delete req.session.error;
  delete req.session.success;
  delete req.session.notice;

  if (err) res.locals.error = err;
  if (msg) res.locals.notice = msg;
  if (success) res.locals.success = success;

  next();
});

passport.use(new LocalStrategy(
    function(username, password, done) {
    	var user=users.find({'account':username});
    	console.log(user.length);
        if (user.length==0) {
          return done(null, false,{ message: 'Incorrect username.'})
        }
        if (password !=user[0].pwd   ) {
          return done(null, false,{ message: 'Incorrect password.' })
        }
        return done(null, user[0])
      })
)

passport.serializeUser(function(user, done) {
  done(null, user.account);
});

passport.deserializeUser(function(username, done) {
    var user=users.find({'account':username});
    if (user.length==0) { return done(null,false); }
    done(null, user);
});  

app.get('/',function(req, res){ //我們要處理URL為 "/" 的HTTP GET請求
    res.render('index',{
    	/*introHead:'Welcome to CanStagingA',
    	intro: 'we help you rapidly evaluate five different kinds of cancer using TNM model.',
    	create:'Create an account',
    	signin:'sign in'*/
        wrong_info: null
    });
});


app.get('/test',function(req, res){ //我們要處理URL為 "/" 的HTTP GET請求
    res.render('test');
});
app.get('/language', function(req,res){
    var lan = req.query.val;
    i18nn.setLocale(lan);
    res.setLocale(i18nn.getLocale());
    res.render('index',{
    	/*introHead:'Welcome to CanStagingA',
    	intro: 'we help you rapidly evaluate five different kinds of cancer using TNM model.',
    	create:'Create an account',
    	signin:'sign in'*/
        wrong_info: null
    });
});

app.get('/main', function (req, res) {    
	if(req.user!=undefined){
        acc = req.user[0].account;   
        console.log(req.user[0].account);
        res.render('main', {
            user: acc,
            aType: users.find({'account':acc})[0].acctype  
        });
    }else{
        acc=req.query.name;
        console.log(req.query.name);
        res.render('main', {
            user: acc,
            aType: users.find({'account':acc})[0].acctype 
        });
    }
});

users.insert({
    account: "admin@admin",
    pwd: "admin",
    acctype: "admin",
    time: moment().format('YYYY-MM-DD HH:mm:ss Z')
});

app.get('/myaccount', function (req, res) {   
    res.render('myaccount', {
        user: req.query.name,
        acctype: users.find({'account':req.query.name})[0].acctype,
        aType: users.find({'account':req.query.name})[0].acctype,
        wrong_info: null
    });  
});

app.get('/manage_user',function(req,res){
    if (req.query.name=="admin@admin"){
        var n = users.find();
        res.render('admin_myaccount', {
            user:req.query.name,
            aType: users.find({'account':req.query.name})[0].acctype,
            log_data:n
        });
    }
});

app.post('/edit_acc', function(req,res){

    if (req.body.acctype=="Clinician")
        users.find({'account':req.body.acc})[0].acctype="Clinician";
    else if (req.body.acctype=="Technician")
        users.find({'account':req.body.acc})[0].acctype="Technician";
    
    users.update(users.find({'account':req.body.acc})[0]);
    
    if (req.body.del=="T"){
        users.remove(users.find({'account':req.body.acc})[0]);
        data.remove(data.find({'owner':req.body.acc}));
    }

    res.render('admin_myaccount', {
        user:req.query.name,
        log_data:users.find(),
        aType: users.find({'account':req.query.name})[0].acctype
    });
});


app.post('/change_pwd', function (req, res) {
    acc = req.query.name;
    var n = users.find({'account':acc});

    if ((req.body.newpwd==req.body.repeat_newpwd)&&
        (n[0].pwd==req.body.oldpwd)) {
        
        n[0].pwd = req.body.newpwd;
        users.update(n[0]);

        res.render('main',{
            user:acc,
            aType: users.find({'account':acc})[0].acctype            
        });
    }
    else{
        res.render('myaccount', {
            user: req.query.name,
            aType: users.find({'account':req.query.name})[0].acctype,
            acctype: users.find({'account':req.query.name})[0].acctype,
            wrong_info: "Incorrect information"
        });    
    }

});

app.post('/signin', passport.authenticate('local', {
    successRedirect: '/main',
    failureRedirect: '/',
    failureFlash : true 
}));


app.get('/logout', function(req, res, next) {
  var name = req.query.name;
  console.log("LOGGIN OUT " + name);
  req.logout();
  req.session.save(function (err) {
        if (err) {
            return next(err);
        }
        req.session.notice = "You have successfully been logged out " + name + "!";
        
        res.redirect('/');
    });
});


/*app.post('/signin', function (req, res) {
	acc = req.body.user
    var n = users.find({'account':acc});
    if(n.length!=0&&req.body.password==n[0].pwd){
        res.render('main',{
            user: acc
        });
    }else{
        res.render('index',{
            introH:'Welcome to CanStagingA',
            intro: 'we help you rapidly evaluate five different kinds of cancer using TNM model.',
            create:'Sign in'
        });
    }
	
});*/

app.post('/register', function (req, res) {
    acc = req.body.user
    var n = users.find({'account':acc});    
    
    if(n.length==0){
        if (req.body.password==req.body.confirmedPassword){
            var now = moment()
            var formatted = now.format('YYYY-MM-DD HH:mm:ss Z')
            console.log(formatted)
            users.insert({
                account: acc,
                pwd: req.body.password,
                acctype: req.body.accType,
                time: formatted
            });
            res.render('main',{
                user:acc,
                aType: req.body.accType
            });
        }
        else{
            res.render('index',{
                wrong_info: "Incorrect password"
            });
        }
    }else{
        res.render('index',{
            wrong_info: "Existing account"
        });
    }
    
    console.log(acc);
});
var btt = fs.readFileSync('./tnm_tables/Breast/Breast_T.txt').toString().replace(/ /gm,"!").split("\n");
var bnn = fs.readFileSync('./tnm_tables/Breast/Breast_N.txt').toString().replace(/ /gm,"!").split("\n");
var bmm = fs.readFileSync('./tnm_tables/Breast/Breast_M.txt').toString().replace(/ /gm,"!").split("\n");
var bssg = fs.readFileSync('./tnm_tables/Breast/Breast_SG.txt').toString().replace(/ /gm,"!").split("\n");
var bssimg = fs.readFileSync('./tnm_tables/Breast/Breast_SimpG.txt').toString().replace(/ /gm,"!").split("\n");
//var btt = fs.readFileSync('./tnm_tables/Breast/Breast_T.txt').toString().replace(/ /gm,"!").split("\r\n");
//var bnn = fs.readFileSync('./tnm_tables/Breast/Breast_N.txt').toString().replace(/ /gm,"!").split("\r\n");
//var bmm = fs.readFileSync('./tnm_tables/Breast/Breast_M.txt').toString().replace(/ /gm,"!").split("\r\n");
//var bssg = fs.readFileSync('./tnm_tables/Breast/Breast_SG.txt').toString().replace(/ /gm,"!").split("\r\n");
//var bssimg = fs.readFileSync('./tnm_tables/Breast/Breast_SimpG.txt').toString().replace(/ /gm,"!").split("\r\n");

var cett = fs.readFileSync('./tnm_tables/Cervix/Cervix_T.txt').toString().replace(/ /gm,"!").split("\n");
var cenn = fs.readFileSync('./tnm_tables/Cervix/Cervix_N.txt').toString().replace(/ /gm,"!").split("\n");
var cemm = fs.readFileSync('./tnm_tables/Cervix/Cervix_M.txt').toString().replace(/ /gm,"!").split("\n");
var cessg = fs.readFileSync('./tnm_tables/Cervix/Cervix_SG.txt').toString().replace(/ /gm,"!").split("\n");
var cessimg = fs.readFileSync('./tnm_tables/Cervix/Cervix_SimpG.txt').toString().replace(/ /gm,"!").split("\n");

var cott = fs.readFileSync('./tnm_tables/Colorectal/Colorectal_T.txt').toString().replace(/ /gm,"!").split("\n");
var conn = fs.readFileSync('./tnm_tables/Colorectal/Colorectal_N.txt').toString().replace(/ /gm,"!").split("\n");
var comm = fs.readFileSync('./tnm_tables/Colorectal/Colorectal_M.txt').toString().replace(/ /gm,"!").split("\n");
var cossg = fs.readFileSync('./tnm_tables/Colorectal/Colorectal_SG.txt').toString().replace(/ /gm,"!").split("\n");
var cossimg = fs.readFileSync('./tnm_tables/Colorectal/Colorectal_SimpG.txt').toString().replace(/ /gm,"!").split("\n");

var ltt = fs.readFileSync('./tnm_tables/Lung/Lung_T.txt').toString().replace(/ /gm,"!").split("\n");
var lnn = fs.readFileSync('./tnm_tables/Lung/Lung_N.txt').toString().replace(/ /gm,"!").split("\n");
var lmm = fs.readFileSync('./tnm_tables/Lung/Lung_M.txt').toString().replace(/ /gm,"!").split("\n");
var lssg = fs.readFileSync('./tnm_tables/Lung/Lung_SG.txt').toString().replace(/ /gm,"!").split("\n");
var lssimg = fs.readFileSync('./tnm_tables/Lung/Lung_SimpG.txt').toString().replace(/ /gm,"!").split("\n");

var ptt = fs.readFileSync('./tnm_tables/Prostate/Prostate_T.txt').toString().replace(/ /gm,"!").split("\n");
var pnn = fs.readFileSync('./tnm_tables/Prostate/Prostate_N.txt').toString().replace(/ /gm,"!").split("\n");
var pmm = fs.readFileSync('./tnm_tables/Prostate/Prostate_M.txt').toString().replace(/ /gm,"!").split("\n");
var pssg = fs.readFileSync('./tnm_tables/Prostate/Prostate_SG.txt').toString().replace(/ /gm,"!").split("\n");
var pssimg = fs.readFileSync('./tnm_tables/Prostate/Prostate_SimpG.txt').toString().replace(/ /gm,"!").split("\n");

app.get('/breast', function (req,res){
    acc = req.query.name;
    console.log(acc);
    console.log(JSON.stringify(btt));
    //res.send('alert('+btt+')');
	res.render('template',{
        user:acc,
        aType: users.find({'account':acc})[0].acctype,
        mode:'1',

        am:[-1,-1,8,0,0,0,-1,0,5,0,0,0,0,-1,3],
        qf:[0,0,['','0~0','0.11~0.5','0.51~1','1.01~2','0~0.1','2.01~5','5.01~15.9'],
            0,0,0,0,0,['',1,2,3,4],0,0,0,0,0,['','No distant metastases','Distant metastases']],

		labelList:['Home','Breast','Cervix','Colorectal','Lung','Prostate'],
        labela:['/main?name='+acc,
                '/breast?name='+acc,
                '/cervix?name='+acc,
                '/colorectal?name='+acc,
                '/lung?name='+acc,
                '/prostate?name='+acc],

		q:[ 'After adjuvant treatment',
            'Information source of T is pathlogy',
            'Enter size of invasive tumour (cm)',
			'Extension to chest wall (not including only pectoralis muscle',
			'Ulceration/ipsilateral satellite nodules/oedema (incl. peau d"orange) of skin',
			'Inflammatory carcinoma',

            'Information source of N is pathlogy',
			'Micrometastases only: none>2.0mm (but: >0.2mm or more than 200 cells)',
			'Number of positive axillary nodes (at least one deposit > 2.0mm)',
			'Metastases in internal mammary nodes with metastases in sentinel lymphnode(not clinically detected)',
			'Metastases in clinically detected internal mammary nodes',
			'Metastases in infraclavicular nodes',
			'Metastases in supraclavicular nodes',

            'Information source of M is pathlogy',
			'Distant meastases'],

        ansq:[   'ay','tpc','ans[1]','ans[2]','ans[3]','ans[4]',
                 'npc','ans[5]','ans[6]','ans[7]','ans[8]','ans[9]','ans[10]',
                 'mpc','ans[11]'],

        tt:     btt,
        nn:     bnn,
        mm:     bmm,
        ssg:    bssg,
        ssimg:  bssimg
	});
});

app.get('/cervix', function (req,res){
    acc = req.query.name;
    console.log(acc);
    res.render('template',{
        user:acc,
        aType: users.find({'account':acc})[0].acctype,
        mode:'2',

        am:[-1,-1,0,0,4,4,0,0,0,0,0,-1,3,-1,3],
        qf:[0,0,0,0,['','<=3mm in depth AND <=7mm in horizontal spread',
            '>3mm but <=5mm in depth AND <=7mm in horizontal spread',
            '>5mm in depth OR >7mm in horizontal spread'],
            ['','No','Yes, <= 4.0cm','Yes, > 4cm'],
            0,0,0,0,0,0,['',1,2],0,
            ['','No distant (or paraortic nodes) metastases','Distant (or paraortic nodes) metastases']],

        labelList:['Home','Breast','Cervix','Colorectal','Lung','Prostate'],
        labela:['/main?name='+acc,
                '/breast?name='+acc,
                '/cervix?name='+acc,
                '/colorectal?name='+acc,
                '/lung?name='+acc,
                '/prostate?name='+acc],

        q:[ 'After adjuvant treatment',
            'Information source of T is pathlogy',
            'Tumour invades beyond uterus',
            'Microscopic lesion only',
            'Stromal invasion',
            'Clinically visible lesion',
            'Parametrial involvement',
            'Carcinoma involves lower 1/3 of vagina',
            'Extension onto pelvic wall',
            'Hydronephrosis or non-functioning kidney',
            'Tumour invades mucosa of bladder or rectum',

            'Information source of N is pathlogy',
            'Number of positive nodes',            

            'Information source of M is pathlogy',
            'Distant meastases'],

        ansq:['ay','tpc','ans[1]','ans[2]','ans[3]','ans[4]','ans[5]','ans[6]','ans[7]','ans[8]','ans[9]',
              'npc','ans[10]',
              'mpc','ans[11]'],

        tt:     cett,
        nn:     cenn,
        mm:     cemm,
        ssg:    cessg,
        ssimg:  cessimg
    });
});

app.get('/colorectal', function (req,res){
    acc = req.query.name;
    console.log(acc);
    res.render('template',{
        user:acc,
        aType: users.find({'account':acc})[0].acctype,
        mode:'3',

        am:[-1,-1,6,-1,6,0,-1,4],
        qf:[0,0,['','Tumour invades submucosa',
             'Tumour invades muscularis propria',
             'Tumour invades subserosa or into non-peritonealized pericolorectal tissues',
             'Tumour perforates visceral peritoneum',
             'Tumour directly invades or is adherent to other organs or structures'],
            0,
            ['','1','2','3','4','5'],0,
            0,
            ['','No distant metastases',
             'Metastases confined to one organ or site (e.g. liver, lung, ovary, non-regional node)',
             'Metastases in more than one organ/site or the peritoneum']],

        labelList:['Home','Breast','Cervix','Colorectal','Lung','Prostate'],
        labela:['/main?name='+acc,
                '/breast?name='+acc,
                '/cervix?name='+acc,
                '/colorectal?name='+acc,
                '/lung?name='+acc,
                '/prostate?name='+acc],

        q:[ 'After adjuvant treatment',
            'Information source of T is pathlogy',
            'Level of invasion',

            'Information source of N is pathlogy',
            'Number of positive nodes',
            'Tumour deposit in subserosa, mesentery, non-peritonealized pericolic or perirectal tissues',

            'Information source of M is pathlogy',
            'Distant metastases'],

        ansq:['ay','tpc','ans[1]','npc','ans[2]','ans[3]','mpc','ans[4]'],

        tt:     cott,
        nn:     conn,
        mm:     comm,
        ssg:    cossg,
        ssimg:  cossimg
    });
});

app.get('/lung', function (req,res){
    acc = req.query.name;
    console.log(acc);
    res.render('template',{
        user:acc,
        aType: users.find({'account':acc})[0].acctype,
        mode:'4',

        am:[-1,-1,7,5,0,0,0,0,0,0,0,0,-1,5,-1,4],

        qf:[0,0,['','0~0','0.01~2','2.01~3','3.01~5','5.01~7','7.01~50'],
            ['','Tumour does NOT invade main bronchus',
             'Tumour involves the main bronchus >= 2cm distal to the carina',
             'Tumour involves the main brochus < 2cm distal to the carina',
             'Tumour involves the carina or trachea'],
             0,0,0,0,0,0,0,0,0,
            ['','No regional lymph node metastases',
             'Metastases to ipsilateral peribronchial AND/OR ipsilateral hilar nodes AND intrapulmonary nodes',
             'Metastases to ipsilateral mediastinal AND/OR subcarinal nodes',
             'Metastases to contralateral mediastinal, contralateral hilar, ipsilateral or contralateral scalene, or supraclavicular nodes'],
            0,['','No distant metastases',
             'Separate tumour nodule(s) in a contralateral lobe tumour with pleural nodes or malignant pleural (or pericardial) effusion',
             'Distant metastases']],

        labelList:['Home','Breast','Cervix','Colorectal','Lung','Prostate'],
        labela:['/main?name='+acc,
                '/breast?name='+acc,
                '/cervix?name='+acc,
                '/colorectal?name='+acc,
                '/lung?name='+acc,
                '/prostate?name='+acc],

        q:[ 'After adjuvant treatment',
            'Information source of T is pathlogy',
            'Enter size of invasive tumour',
            'Bronchial invasion',
            'Invades the visceral pleura',
            'Atelectasis or obstructive pneumonitis (Not entire lung)',
            'Tumour invades any of the following: chest wall or rib; diaphragm; phrenic nerve; mediastinal pleura; parietal pericardium',
            'Separate tumour nodules in same lobe',
            'Atelectasis or obstructive pneumonitis of the entire lung',
            'Vocal cord paralysis',
            'Tumour invades any of the following mediastinum: heart or great vessels; recurrent laryngeal nerve; vertebral body; oesophagus',
            'Separate tumour nodule(s) in a different ipsilateral lobe',

            'Information source of N is pathlogy',
            'Reginal lymph nodes',

            'Information source of M is pathlogy',
            'Distant meastases'],

        ansq:[  'ay','tpc','ans[1]','ans[2]','ans[3]','ans[4]','ans[5]',
                'ans[6]','ans[7]','ans[8]','ans[9]','ans[10]',
                'npc','ans[11]',
                'mpc','ans[12]'],

        tt:     ltt,
        nn:     lnn,
        mm:     lmm,
        ssg:    lssg,
        ssimg:  lssimg
    });
});

app.get('/prostate', function (req,res){
    acc = req.query.name;
    console.log(acc);
    res.render('template',{
        user:acc,
        aType: users.find({'account':acc})[0].acctype,
        mode:'5',

        am:[-1,-1,3,7,0,0,0,0,0,0,0,-1,3,-1,3,0,0,0,4,4],

        qf:[0,0,['','Tumour clinically inapparent (not palpable or visible by imaging)',
             'Tumour clinically apparent'],
            ['','Tumour incidental histological finding in = 5% resected tissue',
             'Tumour incidental histological finding >5% resected tissue',
             'Tumour identified by needle biopsy',
             'Tumour involves one 1/2 of one lobe or less',
             'Tumour involves more than half of one lobe',
             'Tumour involves both lobes'],
             0,0,0,0,0,0,0,
            0,['','1','2'],
            0,['','No distant metastases','Distant metastases'],
            0,0,0,
            ['','1~9','10~19','20~999'],
            ['','2~6','7','8~10']],

        labelList:['Home','Breast','Cervix','Colorectal','Lung','Prostate'],
        labela:['/main?name='+acc,
                '/breast?name='+acc,
                '/cervix?name='+acc,
                '/colorectal?name='+acc,
                '/lung?name='+acc,
                '/prostate?name='+acc],

        q:[ 'After adjuvant treatment',
            'Information source of T is pathlogy',
            'Apearence',
            'Extension',
            'Throufh capsule',
            'Seminal vescicles',
            'Bladder neck (microscopic only)',
            'Bladder',
            'External sphincter',
            'Rectum or levator muscles',
            'Pelvic wall',

            'Information source of N is pathlogy',
            'Number of positive nodes',

            'Information source of M is pathlogy',
            'Distant metastases',
            'Metastases to bone',
            'Metastases to non-regional nodes',
            'Metastases to other site',
            'SG (PSA value)',
            'SG (Gleason score)'],

        ansq:[  'ay','tpc','ans[1]','ans[2]','ans[3]','ans[4]','ans[5]','ans[6]','ans[7]','ans[8]','ans[9]',
                'npc','ans[10]','mpc','ans[11]','ans[12]','ans[13]','ans[14]','ans[15]','ans[16]'],

        tt:     ptt,
        nn:     pnn,
        mm:     pmm,
        ssg:    pssg,
        ssimg:  pssimg
    });
});


app.get('/log', function (req,res){

    if (req.query.name=="admin@admin"){
        var n = data.find();
        res.render('log',{
            user:req.query.name,
            log_data:n,
            aType: users.find({'account':req.query.name})[0].acctype
        });
    }
    else{
        var n = data.find({'owner':req.query.name});
        res.render('log',{
            user:req.query.name,
            aType: users.find({'account':req.query.name})[0].acctype,
            log_data:n
        });
    }
})

app.post('/manage_rec', function (req,res){
    var n = data.find({'owner':req.body.owner});
    for (var i=0; i<n.length; i++){
        if (n[i].ID==req.body.ID){
            if (req.body.del=="T"){
                data.remove(n[i]);
                break;
            }
            else{
                n[i].Site = req.body.Site;
                n[i].TNM = req.body.TNM;
                n[i].SG = req.body.SG;
                n[i].SimpG = req.body.SimpG;
                n[i].SGRange = req.body.SGRange;
                n[i].Duke = req.body.Duke;
                data.update(n[i]);
                break;                
            }
        }
    }
    
    res.render('log', {
        user:req.query.name,
        aType: users.find({'account':req.query.name})[0].acctype,
        log_data:data.find()
    });
})


app.get('/api/log',function(req,res){
    res.json({
        Record:data.find({'owner':req.query.name})
    });
})


app.post('/api/store', function (req,res){
    if (req.body.pwd==users.find({'account':req.body.name})[0].pwd){
        data.insert({
            owner: req.body.name,
            ID: req.body.ID,
            time: moment().format('YYYY-MM-DD HH:mm:ss Z'),
            Site: req.body.Site,
            TNM: req.body.TNM,
            SG: req.body.SG,
            SimpG: req.body.SimpG,
            SGRange: req.body.SGrange,
            Duke: req.body.Duke
        });
        res.json({
            Message: "done"
        });        
    }
    else{
        res.json({
            Message: "Incorrect account/password"
        });           
    }
});

app.get('/upload', function (req,res){    
    res.render('upload',{
        user:req.query.name,
        aType: users.find({'account':req.query.name})[0].acctype,
        btt:     btt,
        bnn:     bnn,
        bmm:     bmm,
        bssg:    bssg,
        bssimg:  bssimg,
        cett:     cett,
        cenn:     cenn,
        cemm:     cemm,
        cessg:    cessg,
        cessimg:  cessimg,
        cott:     cott,
        conn:     conn,
        comm:     comm,
        cossg:    cossg,
        cossimg:  cossimg,
        ptt:     ptt,
        pnn:     pnn,
        pmm:     pmm,
        pssg:    pssg,
        pssimg:  pssimg,
        ltt:     ltt,
        lnn:     lnn,
        lmm:     lmm,
        lssg:    lssg,
        lssimg:  lssimg
    });
})

app.post('/file-upload-batch', function (req,res){      
    res.send('{"success":"OK!"}');
})

app.post('/store', function (req,res){
    var site;
    
    var now = moment()
    if(req.body.datac==1||req.body.datac=='0'){
        var t = now.format('YYYY-MM-DD HH:mm:ss Z');
        if (req.body.Site==1)
            site = "Breast";
        else if (req.body.Site==2)
            site = "Cervix";
        else if (req.body.Site==3)
            site = "Colorectal";
        else if (req.body.Site==4)
            site = "Lung";
        else if (req.body.Site==5)
            site = "Prostate";
        var n = data.find({'owner':req.query.name});
        var idd;
        if (req.body.ID==""||req.body.ID=="-")
            idd = n.length;
        else
            idd =req.body.ID;
            data.insert({
                owner: req.query.name,
                ID: idd,
                time: t,
                Site: site,
                TNM: req.body.TNM,
                SG: req.body.SG,
                SimpG: req.body.SimpG,
                SGRange: req.body.SGrange,
                Duke: req.body.Duke
            });
    }else{
        for(var i = 0; i<req.body.TNM.length; i++){
            var t = now.format('YYYY-MM-DD HH:mm:ss Z');
            if (req.body.Site[i]==1)
                site = "Breast";
            else if (req.body.Site[i]==2)
                site = "Cervix";
            else if (req.body.Site[i]==3)
                site = "Colorectal";
            else if (req.body.Site[i]==4)
                site = "Lung";
            else if (req.body.Site[i]==5)
                site = "Prostate";
            var n = data.find({'owner':req.query.name});
            var idd;
            if (req.body.ID[i]=="-")
                idd = n.length;
            else
                idd =req.body.ID[i];
            data.insert({
                owner: req.query.name,
                ID: idd,
                time: t,
                Site: site,
                TNM: req.body.TNM[i],
                SG: req.body.SG[i],
                SimpG: req.body.SimpG[i],
                SGRange: req.body.SGrange[i],
                Duke: req.body.Duke[i]
            });
        }
    }
    res.render('main', {
        user: req.query.name,
        aType: users.find({'account':req.query.name})[0].acctype 
    });
});

server.listen(process.env.OPENSHIFT_NODEJS_PORT,process.env.OPENSHIFT_NODEJS_IP,function(){
    console.log('HTTP伺服器在 http://127.0.0.1:3000/ 上運行');
});