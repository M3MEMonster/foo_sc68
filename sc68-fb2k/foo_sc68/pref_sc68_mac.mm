#import "stdafx.h"
#import "pref_sc68_mac.h"
#import "global_config.h"
#include "sc68/file68_opt.h"

extern cfg_int cfg_spr;
extern cfg_int cfg_deftime;
extern cfg_int cfg_asid;
extern cfg_int cfg_ym_model;
extern cfg_int cfg_ym_filter;
extern cfg_int cfg_ym_volmodel;
extern cfg_int cfg_amiga_blend;
extern cfg_bool cfg_amiga_filter;

@interface pref_sc68_mac ()
@property (nonatomic) NSNumber* cfg_spr_mac;
@property (nonatomic) NSNumber* cfg_deftime_mac;
@property (nonatomic) NSNumber* cfg_asid_mac;
@property (nonatomic) NSNumber* cfg_ym_model_mac;
@property (nonatomic) NSNumber* cfg_ym_filter_mac;
@property (nonatomic) NSNumber* cfg_ym_volmodel_mac;
@property (nonatomic) NSNumber* cfg_amiga_blend_mac;
@property (nonatomic) BOOL cfg_amiga_filter_mac;
@end

@implementation pref_sc68_mac
static void construct_combo_from_opt(char* opt_key, NSComboBox* combo) {
    [combo removeAllItems];
    option68_t* opt = option68_get(opt_key, opt68_ALWAYS);
    for (unsigned i = 0; i < opt->sets; i++) {
        if (!strcmp68(opt_key, "ym-engine") && !strcmp68(((char**)opt->set)[i], "dump")) continue;
        [combo addItemWithObjectValue:@(((char**)opt->set)[i])];
        
    }
}
- (void)viewDidLoad {
    [super viewDidLoad];
    NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
    formatter.numberStyle = NSNumberFormatterDecimalStyle;
    formatter.usesGroupingSeparator = NO;
    formatter.allowsFloats = NO;
    formatter.lenient=NO;
    formatter.partialStringValidationEnabled = YES;
    self.sampleRateField.formatter = formatter;
    self.defaultTimeField.formatter=formatter;
    NSInteger spr_value=[self.cfg_spr_mac integerValue];
    self.sampleRateField.integerValue=spr_value;
    self.sampleRateStepper.integerValue=spr_value;
    NSInteger default_time=[self.cfg_deftime_mac integerValue];
    self.defaultTimeField.integerValue=default_time;
    self.defaultTimeStepper.integerValue=default_time;
    [self.ASID removeAllItems];
    [self.ASID addItemsWithObjectValues:@[@"Off",@"On",@"Force"]];
    [self.ASID selectItemAtIndex:(NSInteger)[self.cfg_asid_mac integerValue]];
    construct_combo_from_opt("ym-engine", self.YM_model);
    [self.YM_model selectItemAtIndex:(NSInteger)[self.cfg_ym_model_mac integerValue]];
    construct_combo_from_opt("ym-filter", self.YM_filter);
    [self.YM_filter selectItemAtIndex:(NSInteger)[self.cfg_ym_filter_mac integerValue]];
    construct_combo_from_opt("ym-volmodel", self.YM_volmodel);
    [self.YM_volmodel selectItemAtIndex:(NSInteger)[self.cfg_ym_volmodel_mac integerValue]];
    self.Amiga_blend.doubleValue=[self.cfg_amiga_blend_mac doubleValue];
    self.amigaFilterButton.state=self.cfg_amiga_filter_mac ? NSControlStateValueOn : NSControlStateValueOff;
}

- (instancetype)init {
    self=[self initWithNibName: @"pref_sc68_mac" bundle:[NSBundle bundleForClass: [self class]]];
    [self loadSettings];
    return self;
}

- (void) loadSettings {
    self.cfg_spr_mac=[NSNumber numberWithLong: cfg_spr];
    self.cfg_deftime_mac=[NSNumber numberWithLong: cfg_deftime];
    self.cfg_asid_mac=[NSNumber numberWithLong: cfg_asid];
    self.cfg_ym_model_mac=[NSNumber numberWithLong: cfg_ym_model];
    self.cfg_ym_filter_mac=[NSNumber numberWithLong: cfg_ym_filter];
    self.cfg_ym_volmodel_mac=[NSNumber numberWithLong: cfg_ym_volmodel];
    self.cfg_amiga_blend_mac=[NSNumber numberWithLong: cfg_amiga_blend];
    self.cfg_amiga_filter_mac=cfg_amiga_filter;
}

- (IBAction)onSampleChange:(NSTextField*)sender {
    NSInteger value=sender.integerValue;
    if (value>self.sampleRateStepper.maxValue) value=self.sampleRateStepper.maxValue;
    if (value<self.sampleRateStepper.minValue) value=self.sampleRateStepper.minValue;
    self.sampleRateStepper.integerValue=value;
    sender.integerValue=value;
    cfg_spr=value;
    sc68_cntl(0,SC68_SET_OPT_INT,"sampling-rate",(int)cfg_spr);
}

- (IBAction)onSampleStepperChange:(NSStepper *)sender {
    cfg_spr=sender.integerValue;
    self.sampleRateField.integerValue=sender.integerValue;
    sc68_cntl(0,SC68_SET_OPT_INT,"sampling-rate",(int)cfg_spr);
}

- (IBAction)onDefaultTimeChange:(NSTextField *)sender {
    NSInteger value=sender.integerValue;
    if (value>self.defaultTimeStepper.maxValue) value=self.defaultTimeStepper.maxValue;
    if (value<self.defaultTimeStepper.minValue) value=self.defaultTimeStepper.minValue;
    sender.integerValue=value;
    cfg_deftime=value;
    self.defaultTimeField.integerValue=value;
    sc68_cntl(0,SC68_SET_OPT_INT,"default-time",(int)cfg_deftime);
}

- (IBAction)onDefaultStepperChange:(NSStepper *)sender {
    cfg_deftime=sender.integerValue;
    self.defaultTimeField.integerValue=sender.integerValue;
    sc68_cntl(0,SC68_SET_OPT_INT,"defualt-time",(int)cfg_deftime);
}

- (IBAction)onASIDChange:(NSComboBox *)sender {
    cfg_asid=sender.indexOfSelectedItem;
    sc68_cntl(0,SC68_SET_ASID,(int)cfg_asid);
}

- (IBAction)onYMModelChange:(NSComboBox *)sender {
    cfg_ym_model=sender.indexOfSelectedItem;
    sc68_cntl(0,SC68_SET_OPT_INT,"ym-engine",(int)cfg_ym_model);
}

- (IBAction)onYMFilterChange:(NSComboBox *)sender {
    cfg_ym_filter=sender.indexOfSelectedItem;
    sc68_cntl(0,SC68_SET_OPT_INT,"ym-filter",(int)cfg_ym_filter);
}

- (IBAction)onYMVolModelChange:(NSComboBox *)sender {
    cfg_ym_volmodel=sender.indexOfSelectedItem;
    sc68_cntl(0,SC68_SET_OPT_INT,"ym-volmodel",(int)cfg_ym_volmodel);
}

- (IBAction)onAmigaBlendChange:(NSSlider *)sender {
    cfg_amiga_blend=(int)sender.doubleValue;
    sc68_cntl(0,SC68_SET_OPT_INT,"amiga-blend",(int)cfg_amiga_blend);
}

- (IBAction)onAmigaFilterChange:(NSButton*)sender {
    cfg_amiga_filter=sender.state;
    sc68_cntl(0, SC68_SET_OPT_INT, "amiga-filter", (bool)cfg_amiga_filter);
}

@end

namespace {
class preferences_page_sc68 : public preferences_page {
    public:
        service_ptr instantiate() override {
            return fb2k::wrapNSObject( [ pref_sc68_mac new ] );
        }
        const char * get_name() override {return "SC68 Input";}
        GUID get_guid() override {
            return guid_sc68_pref_page;
        }
        GUID get_parent_guid() override {return guid_input;}
    };
    
    FB2K_SERVICE_FACTORY(preferences_page_sc68);
}
