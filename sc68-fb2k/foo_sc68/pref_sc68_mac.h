
#include <Cocoa/Cocoa.h>

@interface pref_sc68_mac : NSViewController
@property (nonatomic,strong) IBOutlet NSTextField*  sampleRateField;
@property (nonatomic,strong) IBOutlet NSStepper* sampleRateStepper;
@property (nonatomic,strong) IBOutlet NSTextField* defaultTimeField;
@property (nonatomic,strong) IBOutlet NSStepper* defaultTimeStepper;
@property (nonatomic,strong) IBOutlet NSComboBox* ASID;
@property (nonatomic,strong) IBOutlet NSComboBox* YM_model;
@property (nonatomic,strong) IBOutlet NSComboBox* YM_filter;
@property (nonatomic,strong) IBOutlet NSComboBox* YM_volmodel;
@property (nonatomic,strong) IBOutlet NSSlider* Amiga_blend;
@property (nonatomic,strong) IBOutlet NSButton* amigaFilterButton;

- (IBAction)onSampleChange:(NSTextField*)sender;
- (IBAction)onSampleStepperChange:(NSStepper*)sender;
- (IBAction)onDefaultTimeChange:(NSTextField*)sender;
- (IBAction)onDefaultStepperChange:(NSStepper*)sender;
- (IBAction)onASIDChange:(NSComboBox*)sender;
- (IBAction)onYMModelChange:(NSComboBox*)sender;
- (IBAction)onYMFilterChange:(NSComboBox*)sender;
- (IBAction)onYMVolModelChange:(NSComboBox*)sender;
- (IBAction)onAmigaBlendChange:(NSSlider*)sender;
- (IBAction)onAmigaFilterChange:(NSButton*)sender;


@end


